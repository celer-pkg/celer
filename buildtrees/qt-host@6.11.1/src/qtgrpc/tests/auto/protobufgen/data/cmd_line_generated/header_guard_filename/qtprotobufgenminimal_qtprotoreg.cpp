
#include "qtprotobufgenminimal.qpb.h"

#include <QtProtobuf/qprotobufregistration.h>

namespace qt::protobuf {
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarEmpty(qRegisterProtobufType<Empty>);
static bool RegisterQtprotobufgenminimalProtobufTypes = [](){ qRegisterProtobufTypes(); return true; }();
} // namespace qt::protobuf

