/*
 * Level decompiler for Amanita Design's Samorost 3
 * Copyright (C) 2016  Alexey Lysiuk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "atf.h"

#include "binaryfile.h"

namespace S3
{

// Adobe Texture File (ATF) format:
// http://www.adobe.com/devnet/archive/flashruntimes/articles/atf-file-format.html

ATF::ATF(BinaryFile& fs)
{
	if (   'A' != fs.readU8()
		|| 'T' != fs.readU8()
		|| 'F' != fs.readU8())
	{
		throw std::runtime_error("Invalid ATF file");
	}

	m_reserved = fs.readU32<BE>();
	m_version = fs.readU8();

	if (3 != m_version)
	{
		throw std::runtime_error("Unsupported ATF version");
	}

	m_length = fs.readU32<BE>();
	m_format = fs.readU8();

	if (0xD != m_format)
	{
		// TODO: support other formats
		throw std::runtime_error("Unsupported ATF format");
	}

	m_logWidth  = fs.readU8();
	m_logHeight = fs.readU8();
	m_count  = fs.readU8();

	readAlphaCompressedLossy(fs);
}

void ATF::readAlphaCompressedLossy(BinaryFile& fs)
{
/*
	DXT5AlphaDataLength             U32                             Length of DXT5 alpha data in bytes
	DXT5AlphaData                   U8[DXT5AlphaDataLength]         LZMA compressed DXT1 data
	DXT5AlphaImgDataLength          U32                             Length of DXT5 alpha image data in bytes
	DXT5AlphaImageData              U8[DXT5AlphaImgDataLength]      JPEG-XR data (JXRC_FMT_8bppGray)
	DXT5DataLength                  U32                             Length of DXT5 data in bytes
	DXT5Data                        U8[DXT5DataLength]              LZMA compressed DXT5 data
	DXT5ImageDataLength             U32                             Length of DXT5 image data in bytes
	DXT5ImageData                   U8[DXT5ImageDataLength]         JPEG-XR data (JXRC_FMT_16bppBGR565)
	PVRTCTopDataLength              U32                             Length of PVRTC4bpp top data in bytes
	PVRTCTopData                    U8[PVRTCTopDataLength]          LZMA compressed PVRTC top data
	PVRTCBottomDataLength           U32                             Length of PVRTC4bpp bottom data in bytes
	PVRTCBottomData                 U8[PVRTCBottomDataLength]       LZMA compressed PVRTC bottom data
	PVRTCImageDataLength            U32                             Length of PVRTC4bpp image data in bytes
	PVRTCImageData                  U8[PVRTCImageDataLength]        JPEG-XR data (JXRC_FMT_16bppBGR555)
	ETC1TopDataLength               U32                             Length of ETC1 top data in bytes
	ETC1TopData                     U8[ETC1TopDataLength]           LZMA compressed ETC1 top data
	ETC1BottomDataLength            U32                             Length of ETC1 bottom data in bytes
	ETC1BottomData                  U8[ETC1BottomDataLength]        LZMA compressed ETC1 bottom data
	ETC1ImageDataLength             U32                             Length of ETC1 image data in bytes
	ETC1ImageData                   U8[ETC1ImageDataLength]         JPEG-XR data (JXRC_FMT_16bppBGR555)
	ETC2RgbaAlphaTopDataLength      U32                             Length of ETC2RGBA alpha top data in bytes
	ETC2RgbaAlphaTopData            U8[ETC2RgbaAlphaTopDataLength]  LZMA compressed ETC2Rgba alpha top data
	ETC2RgbaAlphaBottomDataLength   U32                             Length of ETC2Rgba alpha bottom data in bytes
	ETC2RgbaAlphaBottomData         U8[ETC2RgbaAlphaBottomData]     LZMA compressed ETC2Rgba alpha bottom data
	ETC2RgbaAlphaImgDataLength      U32                             Length of ETC2Rgba alpha image data in bytes
	ETC2RgbaAlphaImageData          U8[ETC2RgbaAlphaImgDataLength]  JPEG-XR data (JXRC_FMT_8bppGray)
	ETC2RgbaTopDataLength           U32                             Length of ETC2Rgba top data in bytes
	ETC2RgbaTopData                 U8[ETC2RgbaTopDataLength]       LZMA compressed ETC2Rgba top data
	ETC2RgbaModeDataLength          U32                             Length of ETC2Rgba mode data in bytes
	ETC2RgbaModeData                U8[ETC2RgbaModeDataLength]      LZMA compressed ETC2Rgba mode data
	ETC2RgbaBottomDataLength        U32                             Length of ETC2Rgba bottom data in bytes
	ETC2RgbaBottomData              U8[ETC2RgbaBottomDataLength]    LZMA compressed ETC2Rgba bottom data
	ETC2RgbaImageDataLength         U32                             Length of ETC2Rgba image data in bytes
	ETC2RgbaImageData               U8[ETC2RgbaImageDataLength]     JPEG-XR data (JXRC_FMT_24bppBGR)
*/

	TextureData* const entries[] =
	{
		&m_dxt5Alpha,
		&m_dxt5AlphaImage,
		&m_dxt5,
		&m_dxt5Image,
		&m_pvrTCTop,
		&m_pvrTCBottom,
		&m_pvrTCImage,
		&m_etc1Top,
		&m_etc1Bottom,
		&m_etc1Image,
		&m_etc2RgbaAlphaTop,
		&m_etc2RgbaAlphaBottom,
		&m_etc2RgbaAlphaImage,
		&m_etc2RgbaTop,
		&m_etc2RgbaMode,
		&m_etc2RgbaBottom,
		&m_etc2RgbaImage,
	};

	for (uint8_t i = 0; i < m_count; i++)
	{
		for (TextureData* const entry : entries)
		{
			entry->push_back(fs.readBuffer<BE>());
		}
	}
}

} // namespace S3
