
#include "annotation.qpb.h"

#include <QtProtobuf/qprotobufregistration.h>

namespace qtprotobufnamespace::tests {
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarAnnotatedMessage1(qRegisterProtobufType<AnnotatedMessage1>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarAnnotatedMessage2(qRegisterProtobufType<AnnotatedMessage2>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarAnnotatedMessage3(qRegisterProtobufType<AnnotatedMessage3>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarAnnotatedMessage4(qRegisterProtobufType<AnnotatedMessage4>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarAnnotatedMessage5(qRegisterProtobufType<AnnotatedMessage5>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarAnnotatedMessage6(qRegisterProtobufType<AnnotatedMessage6>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarAnnotatedMessage7(qRegisterProtobufType<AnnotatedMessage7>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarAnnotatedMessage8(qRegisterProtobufType<AnnotatedMessage8>);
static QtProtobuf::ProtoTypeRegistrar ProtoTypeRegistrarAnnotatedMessage9(qRegisterProtobufType<AnnotatedMessage9>);
static bool RegisterAnnotationProtobufTypes = [](){ qRegisterProtobufTypes(); return true; }();
} // namespace qtprotobufnamespace::tests

