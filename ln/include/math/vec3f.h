#ifndef I_RTR_MATH_VEC3F
#define I_RTR_MATH_VEC3F

#include <smmintrin.h>
#include <stdalign.h>
#include <math.h>
#include <ostream>

///
/// Obtain given component of SSE value
///
#define l_extract(vec, index) _mm_cvtss_f32(_mm_shuffle_ps(vec, vec, index))

///
/// A 3D vector, utilizing SSE
///
/// Can be also used as a GLSL-like color, for that it has
/// `r`, `g`, `b` getters, which are aliases for `x`, `y`, `z`.
///
/// If you need 2D vector, also use this -- xmm registers are
/// 4-floats wide anyways, so no difference is made.
///
/// Default-initializes to NaN!
///
class vec3f {

    __m128 m_value;

    ///
    /// Initialize vector from SSE value
    ///
    /// Highest component must be 0, otherwise dot product
    /// will return wrong stuff.
    ///
    explicit vec3f(__m128 value) :m_value(value) {}

public:

    ///
    /// Zero-initialize the vector
    ///
    inline vec3f() {
        float f_0 = NAN;
        m_value = _mm_load1_ps(&f_0);
    }

    ///
    /// Initialize with same values over all coordinates
    ///
    explicit vec3f(float value) :vec3f(value, value, value) {}

    ///
    /// Initialize the vector from  
    ///
    inline vec3f(float x, float y, float z = 0.0f, float w = 0.0f) {
        alignas(16) float f_args[4] = { x, y, z, w };
        m_value = _mm_load_ps(f_args);
    }

#define DECLARE_GETTER(name, index)\
        inline float name() const { return l_extract(m_value, index); }

    // x(), y(), z() getters
    DECLARE_GETTER(x, 0)
    DECLARE_GETTER(y, 1)
    DECLARE_GETTER(z, 2)
    DECLARE_GETTER(w, 3)

    // Aliases for using this as rgb
    DECLARE_GETTER(r, 0)
    DECLARE_GETTER(g, 1)
    DECLARE_GETTER(b, 2)
    DECLARE_GETTER(a, 3)

#undef DECLARE_GETTER

    ///
    /// Compute dot product
    ///
    ///     a.x * b.x + a.y * b.y + a.z * b.z
    ///
    inline static float dot(vec3f a, vec3f b) {
        // Use specialized SSE4.1 instruction for that
        return l_extract(_mm_dp_ps(a.m_value, b.m_value, 0b11110001), 0);
    }

    ///
    /// Compute cross product
    ///
    ///     ⌈ a.y * b.z - a.z * b.y ⌉
    ///     │ a.z * b.x - a.x * b.z │
    ///     ⌊ a.x * b.y - a.y * b.x ⌋
    ///
    inline static vec3f cross(vec3f a, vec3f b) {

        // Naive implementation
        /*return vec3f(
            a.y() * b.z() - a.z() * b.y(),
            a.z() * b.x() - a.x() * b.z(),
            a.x() * b.y() - a.y() * b.x()
        );*/

        const int shuf_yzx = _MM_SHUFFLE(/*unused*/ 3, /*x*/ 0, /*z*/ 2, /*y*/ 1);
        const int shuf_zxy = _MM_SHUFFLE(/*unused*/ 3, /*y*/ 1, /*x*/ 0, /*z*/ 2);

        __m128 v_a_yzx = _mm_shuffle_ps(a.m_value, a.m_value, shuf_yzx),
               v_b_yzx = _mm_shuffle_ps(b.m_value, b.m_value, shuf_yzx),
               v_a_zxy = _mm_shuffle_ps(a.m_value, a.m_value, shuf_zxy),
               v_b_zxy = _mm_shuffle_ps(b.m_value, b.m_value, shuf_zxy);

        return vec3f(_mm_sub_ps(
            _mm_mul_ps(v_a_yzx, v_b_zxy), _mm_mul_ps(v_a_zxy, v_b_yzx)
        ));
    }

    ///
    /// Compute componentwise maximum
    ///
    static vec3f max(vec3f a, vec3f b) {
        return vec3f(_mm_max_ps(a.m_value, b.m_value));
    }

    ///
    /// Compute componentwise minimum
    ///
    static vec3f min(vec3f a, vec3f b) {
        return vec3f(_mm_min_ps(a.m_value, b.m_value));
    }

    ///
    /// Squared length
    ///
    inline float len2() const {
        return dot(*this, *this);
    }

    ///
    /// Vector length. Uses `sqrt`, so better use `len2()` if possible
    ///
    inline float len() const {
        return sqrtf(len2());
    }

    ///
    /// Return normalized vector
    ///
    inline vec3f norm() const {
        float s_len2 = len2();
        __m128 v_coeff = _mm_rsqrt_ps(_mm_load1_ps(&s_len2));
        return vec3f(_mm_mul_ps(m_value, v_coeff));
    }

    inline vec3f operator+(vec3f o) const {
        return vec3f(_mm_add_ps(m_value, o.m_value));
    }

    inline vec3f operator-(vec3f o) const {
        return vec3f(_mm_sub_ps(m_value, o.m_value));
    }

    inline vec3f operator*(vec3f o) const {
        return vec3f(_mm_mul_ps(m_value, o.m_value));
    }

    inline vec3f operator/(vec3f o) const {
        return vec3f(_mm_div_ps(m_value, o.m_value));
    }

    inline vec3f operator*(float k) const {
        return vec3f(_mm_mul_ps(m_value, _mm_load1_ps(&k)));
    }

    inline vec3f operator/(float k) const {
        return vec3f(_mm_div_ps(m_value, _mm_load1_ps(&k)));
    }

    inline vec3f& operator+=(vec3f o) {
        m_value = _mm_add_ps(m_value, o.m_value);
        return *this;
    }

    inline vec3f& operator-=(vec3f o) {
        m_value = _mm_sub_ps(m_value, o.m_value);
        return *this;
    }

    inline vec3f& operator*=(vec3f o) {
        m_value = _mm_mul_ps(m_value, o.m_value);
        return *this;
    }

    inline vec3f& operator/=(vec3f o) {
        m_value = _mm_div_ps(m_value, o.m_value);
        return *this;
    }

    inline vec3f& operator*=(float k) {
        m_value = _mm_mul_ps(m_value, _mm_load1_ps(&k));
        return *this;
    }

    inline vec3f& operator/=(float k) {
        m_value = _mm_div_ps(m_value, _mm_load1_ps(&k));
        return *this;
    }

    inline vec3f operator-() const {
        float c_0 = 0;
        return vec3f(_mm_sub_ps(_mm_load1_ps(&c_0), m_value));
    } 
};

inline vec3f operator*(float k, vec3f v) { return v * k; }

inline std::ostream& operator<<(std::ostream &out, vec3f vec) {
    return out << "{ " << vec.x() << ", " << vec.y() << ", " << vec.z() << " }";
}

typedef vec3f rgbf;

#undef l_extract


#endif
