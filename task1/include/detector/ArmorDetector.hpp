#pragma once
#include <vector>
#include <memory>
#include "../types/Armor.hpp"
#include "../types/Params.hpp"
#include "LightBarDetector.hpp"

class ArmorDetector {
public:
    ArmorDetector(std::shared_ptr<LightBarDetector> lightbar_detector,
                  const ArmorParams& params);
    
    // 检测装甲板
    std::vector<Armor> detect(const cv::Mat& frame);
    
    // 设置颜色（红队/蓝队）
    void setEnemyColor(bool is_red);
    
    // 获取检测结果
    const std::vector<Armor>& getArmors() const { return armors_; }
    
private:
    std::shared_ptr<LightBarDetector> lightbar_detector_;
    ArmorParams params_;
    std::vector<Armor> armors_;
    bool enemy_is_red_;
    
    // 私有方法
    std::vector<Armor> pairLightBars(const std::vector<LightBar>& lightbars);
    bool isValidArmor(const LightBar& left, const LightBar& right);
    Armor createArmor(const LightBar& left, const LightBar& right);
};