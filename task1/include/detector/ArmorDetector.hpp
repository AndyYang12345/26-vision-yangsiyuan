#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
#include "../types/Armor.hpp"
#include "../types/Params.hpp"

class ArmorDetector {
public:
    explicit ArmorDetector(const ArmorParams& params = ArmorParams());

    std::vector<Armor> match(const std::vector<LightBar>& lightbars) const;
    void updateParams(const ArmorParams& params);

private:
    ArmorParams params_;

    Armor buildArmor(const LightBar& left, const LightBar& right) const;
    bool isValidPair(const LightBar& left, const LightBar& right) const;
};
