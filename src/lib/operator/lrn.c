#include <operator/lrn.h>

#include <stdint.h>
#include <stdbool.h>

void ONNC_RUNTIME_lrn_float(
  void * restrict onnc_runtime_context
  ,const float * restrict input_X
  ,int32_t input_X_ndim, const int32_t * restrict input_X_dims
  ,float * restrict output_Y
  ,int32_t output_Y_ndim, const int32_t * restrict output_Y_dims
  ,float alpha
  ,float beta
  ,float bias
  ,int32_t size
) {
}
