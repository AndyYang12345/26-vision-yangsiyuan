#include "solver/PnPSolver.hpp"
#include <limits>

PnPSolver::PnPSolver(const cv::Mat& camera_matrix,
                     const cv::Mat& dist_coeffs,
                     float small_armor_width,
                     float small_armor_height,
                     float large_armor_width,
                     float large_armor_height)
    : camera_matrix_(camera_matrix.clone())
    , dist_coeffs_(dist_coeffs.clone())
    , small_w_(small_armor_width)
    , small_h_(small_armor_height)
    , large_w_(large_armor_width)
    , large_h_(large_armor_height) {}

std::vector<cv::Point3f> PnPSolver::get3DPoints(ArmorSize size) const {
    float w = (size == LARGE_ARMOR) ? large_w_ : small_w_;
    float h = (size == LARGE_ARMOR) ? large_h_ : small_h_;

    const float half_w = w * 0.5f;
    const float half_h = h * 0.5f;
    return {
        {-half_w,  half_h, 0.0f},  // 左上
        {-half_w, -half_h, 0.0f},  // 左下
        { half_w, -half_h, 0.0f},  // 右下
        { half_w,  half_h, 0.0f}   // 右上
    };
}

bool PnPSolver::solve(const Armor& armor,
                      cv::Mat& rvec,
                      cv::Mat& tvec,
                      bool use_extrinsic_guess) const {
    if (armor.corners.size() != 4 || camera_matrix_.empty()) {
        return false;
    }

    std::vector<cv::Point3f> object_points = get3DPoints(armor.size);
    std::vector<cv::Point2f> image_points = {
        armor.corners[0],  // 左上
        armor.corners[3],  // 左下
        armor.corners[2],  // 右下
        armor.corners[1]   // 右上
    };

    std::vector<cv::Mat> rvecs;
    std::vector<cv::Mat> tvecs;
    std::vector<double> reproj_errors;
    bool success = cv::solvePnPGeneric(
        object_points,
        image_points,
        camera_matrix_,
        dist_coeffs_,
        rvecs,
        tvecs,
        use_extrinsic_guess,
        cv::SOLVEPNP_IPPE,
        cv::noArray(),
        cv::noArray(),
        reproj_errors
    );

    if (!success || rvecs.empty()) {
        success = cv::solvePnP(
            object_points,
            image_points,
            camera_matrix_,
            dist_coeffs_,
            rvec,
            tvec,
            use_extrinsic_guess,
            cv::SOLVEPNP_SQPNP
        );
        return success;
    }

    auto axesMatchImage = [&](const cv::Mat& rvec_candidate,
                              const cv::Mat& tvec_candidate) {
        const float axis_len = 20.0f;
        std::vector<cv::Point3f> axis_points = {
            {0.0f, 0.0f, 0.0f},
            {axis_len, 0.0f, 0.0f},
            {0.0f, axis_len, 0.0f},
            {0.0f, 0.0f, axis_len}
        };
        std::vector<cv::Point2f> projected;
        cv::projectPoints(axis_points, rvec_candidate, tvec_candidate,
                          camera_matrix_, dist_coeffs_, projected);
        if (projected.size() != 4) {
            return false;
        }
        const cv::Point2f& origin = projected[0];
        const cv::Point2f& x_end = projected[1];
        const cv::Point2f& y_end = projected[2];
        const bool x_right = (x_end.x - origin.x) > 0.0f;
        const bool y_up = (y_end.y - origin.y) < 0.0f;
        return x_right && y_up;
    };

    int best_idx = -1;
    double best_err = std::numeric_limits<double>::max();
    for (size_t i = 0; i < rvecs.size(); ++i) {
        if (tvecs[i].at<double>(2) <= 0.0) {
            continue;
        }
        cv::Mat rotation;
        cv::Rodrigues(rvecs[i], rotation);
        cv::Mat normal = rotation * (cv::Mat_<double>(3, 1) << 0.0, 0.0, 1.0);
        const double dot = normal.dot(tvecs[i]);
        if (dot <= 0.0) {
            continue;  // 法向朝向相机，拒绝
        }
        if (!axesMatchImage(rvecs[i], tvecs[i])) {
            continue;
        }
        const double err = (i < reproj_errors.size()) ? reproj_errors[i] : 0.0;
        if (err < best_err) {
            best_err = err;
            best_idx = static_cast<int>(i);
        }
    }

    if (best_idx < 0) {
        for (size_t i = 0; i < rvecs.size(); ++i) {
            if (tvecs[i].at<double>(2) <= 0.0) {
                continue;
            }
            cv::Mat rotation;
            cv::Rodrigues(rvecs[i], rotation);
            cv::Mat normal = rotation * (cv::Mat_<double>(3, 1) << 0.0, 0.0, 1.0);
            const double dot = normal.dot(tvecs[i]);
            if (dot <= 0.0) {
                continue;
            }
            const double err = (i < reproj_errors.size()) ? reproj_errors[i] : 0.0;
            if (err < best_err) {
                best_err = err;
                best_idx = static_cast<int>(i);
            }
        }
    }

    if (best_idx < 0) {
        for (size_t i = 0; i < rvecs.size(); ++i) {
            const double err = (i < reproj_errors.size()) ? reproj_errors[i] : 0.0;
            if (err < best_err) {
                best_err = err;
                best_idx = static_cast<int>(i);
            }
        }
    }

    rvec = rvecs[best_idx];
    tvec = tvecs[best_idx];

    if (!axesMatchImage(rvec, tvec)) {
        cv::Mat rotation;
        cv::Rodrigues(rvec, rotation);
        cv::Mat rz = (cv::Mat_<double>(3, 3) << -1.0, 0.0, 0.0,
                                                0.0, -1.0, 0.0,
                                                0.0, 0.0, 1.0);
        cv::Mat rotation_flipped = rotation * rz;
        cv::Mat rvec_flipped;
        cv::Rodrigues(rotation_flipped, rvec_flipped);
        if (axesMatchImage(rvec_flipped, tvec)) {
            rvec = rvec_flipped;
        }
    }
    return true;
}

double PnPSolver::evaluateReprojectionError(const Armor& armor,
                                            const cv::Mat& rvec,
                                            const cv::Mat& tvec) const {
    if (armor.corners.size() != 4 || camera_matrix_.empty()) {
        return -1.0;
    }

    std::vector<cv::Point3f> object_points = get3DPoints(armor.size);
    std::vector<cv::Point2f> projected;
    cv::projectPoints(object_points, rvec, tvec, camera_matrix_, dist_coeffs_, projected);

    double error = 0.0;
    std::vector<cv::Point2f> image_points = {
        armor.corners[0],  // 左上
        armor.corners[3],  // 左下
        armor.corners[2],  // 右下
        armor.corners[1]   // 右上
    };
    for (size_t i = 0; i < projected.size() && i < image_points.size(); ++i) {
        error += cv::norm(projected[i] - image_points[i]);
    }
    return error / static_cast<double>(projected.size());
}
