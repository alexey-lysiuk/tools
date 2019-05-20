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

#pragma once
#ifndef S3LVLDEC_ATF_H_INCLUDED
#define S3LVLDEC_ATF_H_INCLUDED

#include <cstdint>
#include <vector>

#include <cstdio>

namespace S3
{

typedef std::vector<uint8_t> ByteArray;

class BinaryFile;

class ATF
{
public:
	ATF() = default;
	explicit ATF(BinaryFile& fs);

private:
	uint8_t m_version = 0;
	uint8_t m_format = 0;
	uint8_t m_count = 0;

	uint8_t m_logWidth = 0;
	uint8_t m_logHeight = 0;

	uint32_t m_reserved = 0;
	uint32_t m_length = 0;

	typedef std::vector<ByteArray> TextureData;

	TextureData m_dxt5Alpha;
	TextureData m_dxt5AlphaImage;
	TextureData m_dxt5;
	TextureData m_dxt5Image;
	TextureData m_pvrTCTop;
	TextureData m_pvrTCBottom;
	TextureData m_pvrTCImage;
	TextureData m_etc1Top;
	TextureData m_etc1Bottom;
	TextureData m_etc1Image;
	TextureData m_etc2RgbaAlphaTop;
	TextureData m_etc2RgbaAlphaBottom;
	TextureData m_etc2RgbaAlphaImage;
	TextureData m_etc2RgbaTop;
	TextureData m_etc2RgbaMode;
	TextureData m_etc2RgbaBottom;
	TextureData m_etc2RgbaImage;

	void readAlphaCompressedLossy(BinaryFile& fs);
};

} // namespace S3

#endif // S3LVLDEC_ATF_H_INCLUDED
