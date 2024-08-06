#include "rendering/ImageSupport.h"
#include "filesystem/File.h"

#include <memory>
#include <cmath>
#include <stdio.h>

namespace l::rendering {
	namespace image {

		image_u8::image_u8() : m_width(0), m_height(0) {}
		image_u8::image_u8(uint32_t width, uint32_t height) : m_width(width), m_height(height) {
			m_pixels.resize(width * height);
		}

		image_u8& image_u8::clear() {
			m_width = m_height = 0;
			m_pixels.clear();
			return *this;
		}

		image_u8& image_u8::init(uint32_t width, uint32_t height) {
			clear();

			m_width = width;
			m_height = height;
			m_pixels.resize(width * height);
			return *this;
		}

		image_u8& image_u8::set_all(const color_quad_u8& p) {
			for (uint32_t i = 0; i < m_pixels.size(); i++)
				m_pixels[i] = p;
			return *this;
		}

		image_u8& image_u8::crop(uint32_t new_width, uint32_t new_height) {
			if ((m_width == new_width) && (m_height == new_height))
				return *this;

			image_u8 new_image(new_width, new_height);

			const uint32_t w = std::min(m_width, new_width);
			const uint32_t h = std::min(m_height, new_height);

			for (uint32_t y = 0; y < h; y++)
				for (uint32_t x = 0; x < w; x++)
					new_image(x, y) = (*this)(x, y);

			return swap(new_image);
		}

		image_u8& image_u8::swap(image_u8& other) {
			std::swap(m_width, other.m_width);
			std::swap(m_height, other.m_height);
			std::swap(m_pixels, other.m_pixels);
			return *this;
		}

		image_u8& image_u8::swizzle(uint32_t r, uint32_t g, uint32_t b, uint32_t a) {
			ASSERT((r | g | b | a) <= 3);
			for (uint32_t y = 0; y < m_height; y++)
			{
				for (uint32_t x = 0; x < m_width; x++)
				{
					color_quad_u8 tmp((*this)(x, y));
					(*this)(x, y).set(tmp[r], tmp[g], tmp[b], tmp[a]);
				}
			}

			return *this;
		}

		bool load_png(const std::vector<unsigned char>& data, image_u8& img) {
			img.clear();

			std::vector<unsigned char> pixels;
			unsigned int w = 0, h = 0;
			lodepng::State state;
			auto result = lodepng_inspect(&w, &h, &state, data.data(), data.size());
			if (result != 0) {
				LOG(LogError) << "Failed to load image header";
				return false;
			}

			unsigned int e = lodepng::decode(pixels, w, h, data);
			if (e != 0)
			{
				LOG(LogError) << "Failed loading PNG file %s";
				return false;
			}

			img.init(w, h);
			memcpy(&img.get_pixels()[0], &pixels[0], w * h * sizeof(uint32_t));

			return true;
		}

		bool save_png(const char* pFilename, const image_u8& img, bool save_alpha) {
			const uint32_t w = img.width();
			const uint32_t h = img.height();

			std::vector<unsigned char> pixels;
			if (save_alpha)
			{
				pixels.resize(w * h * sizeof(color_quad_u8));
				memcpy(&pixels[0], &img.get_pixels()[0], w * h * sizeof(color_quad_u8));
			}
			else
			{
				pixels.resize(w * h * 3);
				unsigned char* pDst = &pixels[0];
				for (uint32_t y = 0; y < h; y++)
					for (uint32_t x = 0; x < w; x++, pDst += 3)
						pDst[0] = img(x, y)[0], pDst[1] = img(x, y)[1], pDst[2] = img(x, y)[2];
			}

			return lodepng::encode(pFilename, pixels, w, h, save_alpha ? LCT_RGBA : LCT_RGB) == 0;
		}

		image_metrics::image_metrics() {
			clear();
		}

		void image_metrics::clear() {
			memset(this, 0, sizeof(*this));
		}

		void image_metrics::compute(const image_u8& a, const image_u8& b, uint32_t first_channel, uint32_t num_channels) {
			const bool average_component_error = true;

			const uint32_t width = std::min(a.width(), b.width());
			const uint32_t height = std::min(a.height(), b.height());

			ASSERT((first_channel < 4U) && (first_channel + num_channels <= 4U));

			// Histogram approach originally due to Charles Bloom.
			double hist[256];
			memset(hist, 0, sizeof(hist));

			for (uint32_t y = 0; y < height; y++)
			{
				for (uint32_t x = 0; x < width; x++)
				{
					const color_quad_u8& ca = a(x, y);
					const color_quad_u8& cb = b(x, y);

					if (!num_channels)
						hist[iabs(ca.get_luma() - cb.get_luma())]++;
					else
					{
						for (uint32_t c = 0; c < num_channels; c++)
							hist[iabs(ca[first_channel + c] - cb[first_channel + c])]++;
					}
				}
			}

			m_max = 0;
			double sum = 0.0f, sum2 = 0.0f;
			for (uint32_t i = 0; i < 256; i++)
			{
				if (!hist[i])
					continue;

				m_max = std::max<double>(m_max, i);

				double x = i * hist[i];

				sum += x;
				sum2 += i * x;
			}

			// See http://richg42.blogspot.com/2016/09/how-to-compute-psnr-from-old-berkeley.html
			double total_values = width * height;

			if (average_component_error)
				total_values *= clamp<uint32_t>(num_channels, 1, 4);

			m_mean = clamp<double>(sum / total_values, 0.0f, 255.0f);
			m_mean_squared = clamp<double>(sum2 / total_values, 0.0f, 255.0f * 255.0f);

			m_root_mean_squared = sqrt(m_mean_squared);

			if (!m_root_mean_squared)
				m_peak_snr = 100.0f;
			else
				m_peak_snr = clamp<double>(log10(255.0f / m_root_mean_squared) * 20.0f, 0.0f, 100.0f);
		}

		bool save_dds(const char* pFilename, uint32_t width, uint32_t height, const void* pBlocks, uint32_t pixel_format_bpp, DXGI_FORMAT dxgi_format, bool srgb, bool force_dx10_header) {
			(void)srgb;

			FILE* pFile = NULL;
#ifdef WIN32
			auto result = fopen_s(&pFile, pFilename, "wb");
			if (!result)
#else
			pFile = fopen64(pFilename, "wb");
			if (!pFile)
#endif
			{
				fprintf(stderr, "Failed creating file %s!\n", pFilename);
				return false;
			}

			fwrite("DDS ", 4, 1, pFile);

			DDSURFACEDESC2 desc;
			memset(&desc, 0, sizeof(desc));

			desc.dwSize = sizeof(desc);
			desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;

			desc.dwWidth = width;
			desc.dwHeight = height;

			desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
			desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);

			desc.ddpfPixelFormat.dwFlags |= DDPF_FOURCC;

			desc.lPitch = (((desc.dwWidth + 3) & ~3) * ((desc.dwHeight + 3) & ~3) * pixel_format_bpp) >> 3;
			desc.dwFlags |= DDSD_LINEARSIZE;

			desc.ddpfPixelFormat.dwRGBBitCount = 0;

			if ((!force_dx10_header) &&
				((dxgi_format == DXGI_FORMAT_BC1_UNORM) ||
					(dxgi_format == DXGI_FORMAT_BC3_UNORM) ||
					(dxgi_format == DXGI_FORMAT_BC4_UNORM) ||
					(dxgi_format == DXGI_FORMAT_BC5_UNORM)))
			{
				if (dxgi_format == DXGI_FORMAT_BC1_UNORM)
					desc.ddpfPixelFormat.dwFourCC = (uint32_t)PIXEL_FMT_FOURCC('D', 'X', 'T', '1');
				else if (dxgi_format == DXGI_FORMAT_BC3_UNORM)
					desc.ddpfPixelFormat.dwFourCC = (uint32_t)PIXEL_FMT_FOURCC('D', 'X', 'T', '5');
				else if (dxgi_format == DXGI_FORMAT_BC4_UNORM)
					desc.ddpfPixelFormat.dwFourCC = (uint32_t)PIXEL_FMT_FOURCC('A', 'T', 'I', '1');
				else if (dxgi_format == DXGI_FORMAT_BC5_UNORM)
					desc.ddpfPixelFormat.dwFourCC = (uint32_t)PIXEL_FMT_FOURCC('A', 'T', 'I', '2');

				fwrite(&desc, sizeof(desc), 1, pFile);
			}
			else
			{
				desc.ddpfPixelFormat.dwFourCC = (uint32_t)PIXEL_FMT_FOURCC('D', 'X', '1', '0');

				fwrite(&desc, sizeof(desc), 1, pFile);

				DDS_HEADER_DXT10 hdr10;
				memset(&hdr10, 0, sizeof(hdr10));

				// Not all tools support DXGI_FORMAT_BC7_UNORM_SRGB (like NVTT), but ddsview in DirectXTex pays attention to it. So not sure what to do here.
				// For best compatibility just write DXGI_FORMAT_BC7_UNORM.
				//hdr10.dxgiFormat = srgb ? DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;
				hdr10.dxgiFormat = dxgi_format; // DXGI_FORMAT_BC7_UNORM;
				hdr10.resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE2D;
				hdr10.arraySize = 1;

				fwrite(&hdr10, sizeof(hdr10), 1, pFile);
			}

			fwrite(pBlocks, desc.lPitch, 1, pFile);

			if (fclose(pFile) == EOF)
			{
				fprintf(stderr, "Failed writing to DDS file %s!\n", pFilename);
				return false;
			}

			return true;
		}

		void strip_extension(std::string& s) {
			for (int32_t i = (int32_t)s.size() - 1; i >= 0; i--)
			{
				if (s[i] == '.')
				{
					s.resize(i);
					break;
				}
			}
		}

		void strip_path(std::string& s) {
			for (int32_t i = (int32_t)s.size() - 1; i >= 0; i--)
			{
				if ((s[i] == '/') || (s[i] == ':') || (s[i] == '\\'))
				{
					s.erase(0, i + 1);
					break;
				}
			}
		}
    }

	image::image_u8 LoadPNG(std::string_view filename) {
		l::rendering::image::image_u8 image;
		l::filesystem::File f(filename);
		f.modeRead().modeBinary();
		if (f.open()) {
			auto count = f.fileSize();
			std::vector<unsigned char> buffer;
			auto actuallyRead = f.read(buffer);

			ASSERT(count == actuallyRead);

			l::rendering::image::load_png(buffer, image);
		}
		return image;
	}
}
