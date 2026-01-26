#include "solver/PnPSolver.hpp"

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
        {-half_w, -half_h, 0.0f},
        { half_w, -half_h, 0.0f},
        { half_w,  half_h, 0.0f},
        {-half_w,  half_h, 0.0f}
    };
}

bool PnPSolver::solve(const Armor& armor,
                      cv::Mat& rvec,
                      cv::Mat& tvec,
                      bool use_extrinsic_guess) {
    if (armor.corners.size() != 4 || camera_matrix_.empty()) {
        return false;
    }

    std::vector<cv::Point3f> object_points = get3DPoints(armor.size);
    std::vector<cv::Point2f> image_points = armor.corners;

    const bool success = cv::solvePnP(
        object_points,
        image_points,
        camera_matrix_,
        dist_coeffs_,
        rvec,
        tvec,
        use_extrinsic_guess,
        cv::SOLVEPNP_ITERATIVE
    );

    return success;
}

double PnPSolver::evaluateReprojectionError(const Armor& armor,
                                            const cv::Mat& rvec,
                                            const cv::Mat& tvec) {
    if (armor.corners.size() != 4 || camera_matrix_.empty()) {
        return -1.0;
    }

    std::vector<cv::Point3f> object_points = get3DPoints(armor.size);
    std::vector<cv::Point2f> projected;
    cv::projectPoints(object_points, rvec, tvec, camera_matrix_, dist_coeffs_, projected);

    double error = 0.0;
    for (size_t i = 0; i < projected.size(); ++i) {
        error += cv::norm(projected[i] - armor.corners[i]);
    }
    return error / static_cast<double>(projected.size());
}
