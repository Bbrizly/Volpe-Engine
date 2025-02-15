#include "BoundingVolume.h"
#include "AABBVolume.h"
#include "SphereVolume.h"
#include <iostream>

void BoundingVolume::PrintBoundingVolumeType(const BoundingVolume* volume) {
    if (!volume) {
        std::cout << "Bounding Volume is NULL" << std::endl;
        return;
    }

    if (dynamic_cast<const AABBVolume*>(volume)) {
        std::cout << "Bounding Volume Type: AABB (Axis-Aligned Bounding Box)" << std::endl;
    }
    else if (dynamic_cast<const SphereVolume*>(volume)) {
        std::cout << "Bounding Volume Type: Sphere" << std::endl;
    }
    else {
        std::cout << "Bounding Volume Type: Unknown" << std::endl;
    }
}
