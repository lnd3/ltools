#pragma once

#include <string_view>
#include <logging/LoggingAll.h>

#include "bc7enc/bc7enc.h"
#include "bc7enc/lodepng.h"
#include "bc7enc/dds_defs.h"

namespace l::rendering {
	namespace image {
		
		inline int iabs(int i) {
			if (i < 0) 
				i = -i; 
			return i; 
		}
		
		inline uint8_t clamp255(int32_t i) {
			return (uint8_t)((i & 0xFFFFFF00U) ? (~(i >> 31)) : i); 
		}
		
		template <typename S>
		inline S clamp(S value, S low, S high) {
			return (value < low) ? low : ((value > high) ? high : value); 
		}

		struct color_quad_u8
		{
			uint8_t m_c[4];

			inline color_quad_u8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
				set(r, g, b, a);
			}

			inline color_quad_u8(uint8_t y = 0, uint8_t a = 255) {
				set(y, a);
			}

			inline color_quad_u8& set(uint8_t y, uint8_t a = 255) {
				m_c[0] = y;
				m_c[1] = y;
				m_c[2] = y;
				m_c[3] = a;
				return *this;
			}

			inline color_quad_u8& set(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
				m_c[0] = r;
				m_c[1] = g;
				m_c[2] = b;
				m_c[3] = a;
				return *this;
			}

			inline uint8_t& operator[] (uint32_t i) { 
				ASSERT(i < 4);
				return m_c[i%4];
			}
			
			inline uint8_t operator[] (uint32_t i) const {
				ASSERT(i < 4);
				return m_c[i%4]; 
			}

			inline int get_luma() const {
				return (13938U * m_c[0] + 46869U * m_c[1] + 4729U * m_c[2] + 32768U) >> 16U;
			} // REC709 weightings
		};

		typedef std::vector<color_quad_u8> color_quad_u8_vec;

		struct block8 {
			uint64_t m_vals[1];
		};

		typedef std::vector<block8> block8_vec;

		struct block16 {
			uint64_t m_vals[2];
		};

		typedef std::vector<block16> block16_vec;

		class image_u8 {
		public:
			image_u8();
			image_u8(uint32_t width, uint32_t height);

			inline const color_quad_u8_vec& get_pixels() const {
				return m_pixels;
			}

			inline color_quad_u8_vec& get_pixels() {
				return m_pixels;
			}

			inline uint32_t width() const {
				return m_width;
			}

			inline uint32_t height() const {
				return m_height;
			}

			inline uint32_t total_pixels() const {
				return m_width * m_height;
			}

			inline color_quad_u8& operator()(uint32_t x, uint32_t y) {
				ASSERT(x < m_width && y < m_height);
				return m_pixels[x + m_width * y];
			}

			inline const color_quad_u8& operator()(uint32_t x, uint32_t y) const {
				ASSERT(x < m_width && y < m_height);
				return m_pixels[x + m_width * y];
			}

			image_u8& clear();
			image_u8& init(uint32_t width, uint32_t height);
			image_u8& set_all(const color_quad_u8& p);
			image_u8& crop(uint32_t new_width, uint32_t new_height);
			image_u8& swap(image_u8& other);

			inline void get_block(uint32_t bx, uint32_t by, uint32_t width, uint32_t height, color_quad_u8* pPixels) {
				ASSERT((bx * width + width) <= m_width);
				ASSERT((by * height + height) <= m_height);

				for (uint32_t y = 0; y < height; y++) {
					memcpy(pPixels + y * width, &(*this)(bx * width, by * height + y), width * sizeof(color_quad_u8));
				}
			}

			inline void set_block(uint32_t bx, uint32_t by, uint32_t width, uint32_t height, const color_quad_u8* pPixels) {
				ASSERT((bx * width + width) <= m_width);
				ASSERT((by * height + height) <= m_height);

				for (uint32_t y = 0; y < height; y++) {
					memcpy(&(*this)(bx * width, by * height + y), pPixels + y * width, width * sizeof(color_quad_u8));
				}
			}

			image_u8& swizzle(uint32_t r, uint32_t g, uint32_t b, uint32_t a);

		private:
			color_quad_u8_vec m_pixels;
			uint32_t m_width, m_height;
		};

		class image_metrics {
		public:
			double m_max, m_mean, m_mean_squared, m_root_mean_squared, m_peak_snr;

			image_metrics();
			void clear();
			void compute(const image_u8& a, const image_u8& b, uint32_t first_channel, uint32_t num_channels);
		};

		bool load_png(const std::vector<unsigned char>& data, image_u8& img);
		bool save_png(const char* pFilename, const image_u8& img, bool save_alpha);
		bool save_dds(const char* pFilename, uint32_t width, uint32_t height, const void* pBlocks, uint32_t pixel_format_bpp, DXGI_FORMAT dxgi_format, bool srgb, bool force_dx10_header);
		void strip_extension(std::string& s);
		void strip_path(std::string& s);
	}

	image::image_u8 LoadPNG(std::string_view filename);

};
