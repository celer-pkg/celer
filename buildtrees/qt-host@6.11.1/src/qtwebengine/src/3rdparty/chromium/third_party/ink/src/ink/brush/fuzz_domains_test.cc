#include "ink/brush/fuzz_domains.h"

#include "fuzztest/fuzztest.h"
#include "ink/brush/brush.h"
#include "ink/brush/brush_family.h"

namespace ink {
namespace {

void ValidBrushGeneratesOnlyValidInstances(const Brush& brush) {
  // ValidBrush() calls Brush::Create().value(), which will CHECK-fail if the
  // result is invalid. So there's nothing else to check.
}
FUZZ_TEST(FuzzDomainsTest, ValidBrushGeneratesOnlyValidInstances)
    .WithDomains(ValidBrush());

void ValidBrushFamilyGeneratesOnlyValidInstances(const BrushFamily& family) {
  // ValidBrushFamily() calls BrushFamily::Create().value(), which will
  // CHECK-fail if the result is invalid. So there's nothing else to check.
}
FUZZ_TEST(FuzzDomainsTest, ValidBrushFamilyGeneratesOnlyValidInstances)
    .WithDomains(ValidBrushFamily());

}  // namespace
}  // namespace ink
