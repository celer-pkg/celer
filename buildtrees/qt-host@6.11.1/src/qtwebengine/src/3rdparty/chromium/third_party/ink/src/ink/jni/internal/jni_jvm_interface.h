// Copyright 2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INK_JNI_INTERNAL_JNI_JVM_INTERFACE_H_
#define INK_JNI_INTERNAL_JNI_JVM_INTERFACE_H_

#include <jni.h>

// This file contains logic to handle caching of JVM classes and methods called
// back to from Ink's JNI code.
//
// The classes and methods are cached lazily the first time they are needed.
// Classes can be looked up with Class[JavaClassName](env). Methods can be
// looked up with Method[JVMClassName][JVMMethodName](env).
//
// Caching classes or the corresponding methods requires holding a global
// reference to the cached classes. JNI_OnUnload should call UnloadJvmInterface
// to clean that up.

namespace ink::jni {

// There's no corresponding OnLoad because loading of this interface is
// done lazily. It's still good practice to have a JNI_OnUnload that cleans up
// global references to the cached classes.
void UnloadJvmInterface(JNIEnv* env);

jclass ClassIllegalStateException(JNIEnv* env);
jclass ClassIllegalArgumentException(JNIEnv* env);
jclass ClassNoSuchElementException(JNIEnv* env);
jclass ClassIndexOutOfBoundsException(JNIEnv* env);
jclass ClassUnsupportedOperationException(JNIEnv* env);
jclass ClassRuntimeException(JNIEnv* env);

jclass ClassImmutableVec(JNIEnv* env);
jmethodID MethodImmutableVecInitXY(JNIEnv* env);

jclass ClassMutableVec(JNIEnv* env);
jmethodID MethodMutableVecSetX(JNIEnv* env);
jmethodID MethodMutableVecSetY(JNIEnv* env);

jclass ClassImmutableBox(JNIEnv* env);
jmethodID MethodImmutableBoxFromTwoPoints(JNIEnv* env);

jclass ClassMutableBox(JNIEnv* env);
jmethodID MethodMutableBoxSetXBounds(JNIEnv* env);
jmethodID MethodMutableBoxSetYBounds(JNIEnv* env);

jclass ClassBoxAccumulator(JNIEnv* env);
jmethodID MethodBoxAccumulatorReset(JNIEnv* env);
jmethodID MethodBoxAccumulatorPopulateFrom(JNIEnv* env);

jclass ClassImmutableParallelogram(JNIEnv* env);
jmethodID MethodImmutableParallelogramFromCenterDimensionsRotationAndShear(
    JNIEnv* env);

jclass ClassMutableParallelogram(JNIEnv* env);
jmethodID MethodMutableParallelogramSetCenterDimensionsRotationAndShear(
    JNIEnv* env);

jclass ClassBrushNative(JNIEnv* env);
jmethodID MethodBrushNativeComposeColorLongFromComponents(JNIEnv* env);

jclass ClassInputToolType(JNIEnv* env);
jmethodID MethodInputToolTypeFromInt(JNIEnv* env);

jclass ClassStrokeInput(JNIEnv* env);
jmethodID MethodStrokeInputUpdate(JNIEnv* env);

}  // namespace ink::jni

#endif  // INK_JNI_INTERNAL_JNI_JVM_INTERFACE_H_
