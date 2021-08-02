#pragma once

#include "Types.h"
#include "PlatformDetection.h"
#include "Logger.h"
#include "Random.h"

#include "Backend/ServerRegistry.h"
#include "Backend/Timer.h"
#include "Backend/MapReduce.h"
#include "Backend/FileHandler.h"
#include "Backend/DLLoader.h"
#include "Backend/FunctionFactory.h"

// Add manually any new generated *.grpc.pb.h files
#include "../GenProtos/cpp/MFR.grpc.pb.h"
#include "../GenProtos/cpp/Utils.grpc.pb.h"
