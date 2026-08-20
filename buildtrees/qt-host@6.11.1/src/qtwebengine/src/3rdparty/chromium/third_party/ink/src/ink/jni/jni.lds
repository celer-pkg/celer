VERS_1.0 {
  # Export JNI symbols.
  global:
    Java_*;
    JNI_*;

  # Hide everything else.
  local:
    *;
};
