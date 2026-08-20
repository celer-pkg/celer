
#include "qtgrpcgen.qpb.h"

#include <QtProtobuf/qprotobufregistration.h>

namespace qtgrpc::tests {
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarIntMessage(qRegisterProtobufType<IntMessage>);
static bool RegisterQtgrpcgenProtobufTypes = [](){ qRegisterProtobufTypes(); return true; }();
} // namespace qtgrpc::tests

