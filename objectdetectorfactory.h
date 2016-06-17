#ifndef OBJECTDETECTORFACTORY_H
#define OBJECTDETECTORFACTORY_H

#include "objectdetectors/objectdetector.h"
#include "objectdetectors/cascadedetector.h"
#include "objectdetectors/hogsvmdetector.h"

#include <string>

using namespace std;

class ObjectDetectorFactory {
public:
    ObjectDetector* make(string name, string model) {
    }
};

#endif // OBJECTDETECTORFACTORY_H
