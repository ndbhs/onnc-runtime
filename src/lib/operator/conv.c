#include <operator/conv.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

static inline bool next_dim(int32_t ndim, int32_t * restrict dim,
                            const int32_t * restrict dim_max) {
  do {
    ndim = ndim - 1;
    dim[ndim] += 1;
    if (dim[ndim] < dim_max[ndim]) {
      return true;
    } else { // reach dimension max
      if (ndim == 0) { // all dimension done
        return false;
      }
      dim[ndim] = 0;
    }
  } while(true);
}

static inline int64_t dim_to_offset(int32_t ndim, const int32_t * restrict dim,
                                    const int32_t * restrict dim_max) {
  int64_t offset = 0;
  int64_t step = 1;
  for (int32_t i = ndim - 1; i >= 0; --i) {
    offset += dim[i] * step;
    step *= dim_max[i];
  }
  return offset;
}

// If it is outside the bounds of the input, use 0.
static inline float get_value_or_zero(int32_t ndim, const int32_t * restrict dim_max,
                                      const float * restrict value, const int32_t * restrict dim) {
  for (int32_t i = 0; i < ndim; ++i) {
    if (dim[i] < 0 || dim[i] >= dim_max[i]) {
      return 0.f;
    }
  }
  return value[dim_to_offset(ndim, dim, dim_max)];
}

void ONNC_RUNTIME_conv_float(
  void * restrict onnc_runtime_context
  ,const float * restrict input_X
  ,int32_t input_X_ndim, const int32_t * restrict input_X_dims
  ,const float * restrict input_W
  ,int32_t input_W_ndim, const int32_t * restrict input_W_dims
  ,const float * restrict input_B
  ,int32_t input_B_ndim, const int32_t * restrict input_B_dims
  ,float * restrict output_Y
  ,int32_t output_Y_ndim, const int32_t * restrict output_Y_dims
  ,const char * restrict auto_pad
  ,int32_t * restrict dilations
  ,int32_t number_of_dilations
  ,int32_t group
  ,int32_t * restrict kernel_shape
  ,int32_t number_of_kernel_shape
  ,int32_t * restrict pads
  ,int32_t number_of_pads
  ,int32_t * restrict strides
  ,int32_t number_of_strides
) {
    int32_t M = input_W_dims[0];
    int32_t C = input_W_dims[1];
    int32_t ndim = input_X_ndim;

    // TODO: type
    int32_t o_dim[ndim];
    memset(o_dim, 0, sizeof(o_dim));
    do { // while o_dim
        int32_t base_dim[ndim];
        base_dim[0] = o_dim[0]; // N
        for (int32_t i = 2; i < ndim; ++i) {
            base_dim[i] = o_dim[i] * strides[i - 2] - pads[i - 2];
        }

        float sum = 0.f;

        int32_t w_dim[ndim];
        memset(w_dim, 0, sizeof(w_dim));
        w_dim[0] = o_dim[1]; // M;
        do { // while w_dim
            if (w_dim[1] == 1) { // all D1 ~ Dn done.
                break;
            }

            int32_t i_dim[ndim];
            i_dim[0] = base_dim[0]; // N
            for (int32_t i = 2; i < ndim; ++i) {
                i_dim[i] = base_dim[i] + w_dim[i] * dilations[i - 2];
            }
            for (int32_t channel = 0; channel < C; ++channel) {
                i_dim[1] = (o_dim[1] * group / M) * C + channel; // input channel <-group-> output channel
                w_dim[1] = channel; // C

                float input = get_value_or_zero(ndim, input_X_dims, input_X, i_dim);
                float weight = get_value_or_zero(ndim, input_W_dims, input_W, w_dim);
                sum += input * weight;
            }
            w_dim[1] = 0; // reset C
        } while (next_dim(ndim, w_dim, input_W_dims));

        if (input_B != NULL) {
            sum += input_B[o_dim[1]];
        }
        output_Y[dim_to_offset(ndim, o_dim, output_Y_dims)] = sum;
    } while (next_dim(ndim, o_dim, output_Y_dims));
}
