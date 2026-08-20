
#include "qtprotobufgen.qpb.h"

#include <QtProtobuf/qprotobufregistration.h>

namespace qt::protobuf {
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarEnumTypes(qRegisterProtobufType<EnumTypes>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarScalarTypes(qRegisterProtobufType<ScalarTypes>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarScalarRepeatedTypes(qRegisterProtobufType<ScalarRepeatedTypes>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarScalarRepeatedNoPackedTypes(qRegisterProtobufType<ScalarRepeatedNoPackedTypes>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarScalarOneOfTypes(qRegisterProtobufType<ScalarOneOfTypes>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarScalarOptionalTypes(qRegisterProtobufType<ScalarOptionalTypes>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarMapScalarTypes(qRegisterProtobufType<MapScalarTypes>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarMessageTypes(qRegisterProtobufType<MessageTypes>);
namespace MessageNestedTypes_QtProtobufNested {
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarNestedMessage(qRegisterProtobufType<NestedMessage>);
} // namespace MessageNestedTypes_QtProtobufNested
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarMessageNestedTypes(qRegisterProtobufType<MessageNestedTypes>);
static bool RegisterQtprotobufgenProtobufTypes = [](){ qRegisterProtobufTypes(); return true; }();
} // namespace qt::protobuf

