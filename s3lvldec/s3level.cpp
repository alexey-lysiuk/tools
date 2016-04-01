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

#include "s3level.h"

#include <cassert>
#include <cstdio>
#include <limits>

#include "binaryfile.h"

namespace S3
{

Level::Level(const char* filename)
{
	if (nullptr != filename)
	{
		open(filename);
	}
}

void Level::open(const char* filename)
{
	close();

	assert(nullptr != filename);
	BinaryFile fs(filename);

	if (   'L' != fs.readU8()
		|| 'V' != fs.readU8()
		|| 'L' != fs.readU8())
	{
		throw std::runtime_error("Invalid level file");
	}

	enum Command : uint8_t
	{
		End = 0,
		Name = 1,
		AtlasTexture = 2,
		Image = 8,
		Sound = 15,
		SoundLoop = 18,
		Player = 20,
		PathData = 25,
		ImageIDs = 30,
		Strings = 31,
		Timelime = 40,
		Buffers = 41,
		Texture = 102,
	};

	while (!fs.endOfFile())
	{
		const uint8_t command = fs.readU8();

		switch (command)
		{
		case Command::End:				return;
		case Command::Name:				loadName(fs);			break;
		case Command::AtlasTexture:		loadAtlasTexture(fs);	break;
		case Command::Image:			loadImage(fs);			break;
		case Command::Sound:			loadSound(fs);			break;
		case Command::SoundLoop:		loadSoundLoop(fs);		break;
		case Command::Player:			loadPlayer(fs);			break;
		case Command::PathData:			loadPathData(fs);		break;
		case Command::ImageIDs:			loadImageIDs(fs);		break;
		case Command::Strings:			loadStrings(fs);		break;
		case Command::Timelime:			loadTimelime(fs);		break;
		case Command::Buffers:			loadBuffers(fs);		break;
		case Command::Texture:			loadTexture(fs);		break;

		default:
			throw std::runtime_error("Invalid command in level data");
			break;
		}
	}
}

void Level::close()
{
	// TODO: clear containers
}

void Level::loadName(BinaryFile& fs)
{
	m_name = fs.readUTF();
}

void Level::loadAtlasTexture(BinaryFile& fs)
{
	const std::string textureName = fs.readUTF();
	const bool hasAtlas = 0 != fs.readU8();

	// TODO: support atlas texture

	const ByteArray buffer = fs.readBuffer();
	//SaveToFile(textureName + ".atf", buffer);
}

void Level::loadImage(BinaryFile& fs)
{
	const std::string name = fs.readUTF();
	const ByteArray buffer = fs.readBuffer();
	//SaveToFile(name + ".swf", buffer);
}

void Level::loadSound(BinaryFile& fs)
{
	const std::string name = m_name + '/' + fs.readUTF();
	m_soundIDs[name] = fs.readU16();

	const ByteArray buffer = fs.readBuffer();
	//SaveToFile(name + ".mp3", buffer);
}

void Level::loadSoundLoop(BinaryFile& fs)
{
	const std::string name = fs.readUTF();
	const ByteArray buffer = fs.readBuffer();
	//SaveToFile(name + ".bin", buffer);
}

void Level::loadPlayer(BinaryFile& fs)
{
	// NOTE: loading plain data, without calculating interpolated values

	uint16_t posCount = fs.readU16();

	const auto readFloats = [&](FloatArray& container)
	{
		container.resize(posCount);

		for (uint16_t i = 0; i < posCount; ++i)
		{
			container[i] = fs.readFloat();
		}
	};

	readFloats(m_player.m_positionsX);
	readFloats(m_player.m_positionsY);
	readFloats(m_player.m_scalesX);
	readFloats(m_player.m_rotations);

	posCount = fs.readU16();

	readFloats(m_player.m_rPositionsX);
	readFloats(m_player.m_rPositionsY);
	readFloats(m_player.m_rRotations);

	const auto readLabels = [&fs](IDToNameMap& container)
	{
		for (uint16_t i = 0, count = fs.readU16(); i < count; ++i)
		{
			const uint16_t frame = fs.readU16();
			container[frame] = fs.readUTF();
		}
	};

	readLabels(m_player.m_labelsAt);
	readLabels(m_player.m_labelsLeft);
	readLabels(m_player.m_labelsRight);
	
	for (uint8_t i = 0, count = fs.readU8(); i < count; i++)
	{
		const int16_t x = fs.readS16();
		const int16_t y = fs.readS16();
		const Position node = { x, y };

		m_player.m_waypoints.push_back(node);
	}

	for (uint8_t i = 0, count = fs.readU8(); i < count; i++)
	{
		const uint8_t type = fs.readU8();
		const uint8_t p1 = fs.readU8();
		const uint8_t p2 = fs.readU8();
		const PathFinderArc arc = { type, p1, p2 };

		m_player.m_arcs.push_back(arc);
	}
}

void Level::loadPathData(BinaryFile& fs)
{
	const uint16_t count = fs.readU16();

	for (uint16_t i = 0; i < count; ++i)
	{
		const float x = fs.readFloat();
		// TODO
	}

	for (uint16_t i = 0; i < count; ++i)
	{
		const float y = fs.readFloat();
		// TODO
	}

	for (uint16_t i = 0; i < count; ++i)
	{
		const float scaleX = fs.readFloat();
		// TODO
	}

	for (uint16_t i = 0; i < count; ++i)
	{
		const float scaleY = fs.readFloat();
		// TODO
	}

	for (uint16_t i = 0; i < count; ++i)
	{
		const float rotation = fs.readFloat();
		// TODO
	}

	for (uint16_t i = 0, labelCount = fs.readU16(); i < labelCount; ++i)
	{
		const uint16_t index = fs.readU16();
		const std::string label = fs.readUTF();
		// TODO
	}

	// TODO: add PathData
}

void Level::loadImageIDs(BinaryFile& fs)
{
	for (uint16_t i = 0, count = fs.readU16(); i < count; ++i)
	{
		const std::string name = fs.readUTF();
		m_imageIDs[name] = fs.readU16();
	}
}

void Level::loadStrings(BinaryFile& fs)
{
	for (uint16_t i = 0, count = fs.readU16(); i < count; ++i)
	{
		const uint16_t index = fs.readU16();
		m_strings[index] = fs.readUTF();
	}
}

void Level::loadTimelime(BinaryFile& fs)
{
	fs.readU32();

	while (true)
	{
		const uint8_t timelineCommand = fs.readU8();

		switch (timelineCommand)
		{
			case 0:
			{
				for (uint32_t i = 0, count = fs.readU32(); i < count; ++i)
				{
					const uint16_t id = fs.readU16();
					const std::string name = fs.readUTF();
					m_namesByIDs[id] = name;
					m_IDsByNames[name] = id;
				}

				m_soundGroupVolumes[0] = 1.0f;

				for (uint32_t i = 0, count = fs.readU32(); i < count; ++i)
				{
					const uint8_t group = fs.readU8();
					const std::string name = fs.readUTF();
					
					m_soundGroups[name] = group;
					m_soundGroupVolumes[group] = 1.0f;
				}

				// Implicitly the last command
				return;
			}

			case 11:
			{
				const uint16_t id = fs.readU16();
				const ByteArray buffer = fs.readBuffer();

				// TODO: create TimelineMemoryBlock
				// spriteDefinitions[id] = new TimelineMemoryBlock(...);
				break;
			}
			
			case 13:
			{
				const uint16_t _loc4_ = fs.readU16();
				const uint8_t bufferNumber = fs.readU8();
				const uint32_t firstIndex = fs.readU32();
				const uint32_t numTriangles = fs.readU32();
				// TODO: create TimelineShape2Definition
				break;
			}
			
			default:
				throw std::runtime_error("Invalid timeline command");
				break;
		}
	}
}

void Level::loadBuffers(BinaryFile& fs)
{
	const uint8_t count = fs.readU8();

	for (uint8_t i = 0; i < count; ++i)
	{
		const ByteArray buffer = fs.read(fs.readU32() * 6 * 4); // ???

		// TODO: parse vertex buffer
	}

	for (uint8_t i = 0; i < count; ++i)
	{
		const ByteArray buffer = fs.read(fs.readU32() * 2); // ???

		// TODO: parse index buffer
	}
}

void Level::loadTexture(BinaryFile& fs)
{
	const std::string textureName = fs.readUTF();
	const uint32_t size = fs.readU32();

	enum XYSize	{ XY_16, XY_32 };

	const auto readRect = [&fs](const XYSize xySize = XY_16)
	{
		const int32_t x = XY_16 == xySize ? fs.readS16() : fs.readS32();
		const int32_t y = XY_16 == xySize ? fs.readS16() : fs.readS32();
		const int16_t width  = fs.readS16();
		const int16_t height = fs.readS16();

		return Rectangle(x, y, width, height);
	};

	enum SubCommand
	{
		End = 0,
		SubTexture = 3,
		MovieName = 4,
		TextureAnim = 5,
		MovieTexRect1 = 6,
		MovieTexRect2 = 7
	};

	while (true)
	{
		switch (fs.readU8())
		{
		case End:
			return;

		case SubTexture:
		{
			const int16_t index = fs.readS16();
			const Rectangle rect = readRect();
			// TODO: create subtexture
			break;
		}

		case MovieName:
		{
			const std::string movieName = fs.readUTF();
			// TODO
			break;
		}

		case TextureAnim:
		{
			for (uint16_t i = 0, count = fs.readU16(); i < count; ++i)
			{
				const uint16_t index = fs.readU16();
				const std::string label = fs.readUTF();
			}
			// TODO: create TextureAnim
			break;
		}

		case MovieTexRect1:
		{
			const Rectangle rect = readRect();
			// TODO: create texture
			break;
		}

		case MovieTexRect2:
		{
			const Rectangle rect1 = readRect();
			const Rectangle rect2 = readRect(XY_32);
			// TODO: create texture
			break;
		}

		default:
			throw std::runtime_error("Invalid texture data");
			break;
		}
	}

	// TODO: create texture
}

} // namespace S3
