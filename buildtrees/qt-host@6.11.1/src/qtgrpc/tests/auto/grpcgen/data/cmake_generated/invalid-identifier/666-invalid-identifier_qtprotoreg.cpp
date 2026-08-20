
#include "666-invalid-identifier.qpb.h"

#include <QtProtobuf/qprotobufregistration.h>

static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarEmpty(qRegisterProtobufType<Empty>);
static bool Register_666_invalid_identifierProtobufTypes = [](){ qRegisterProtobufTypes(); return true; }();

