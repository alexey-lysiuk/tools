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

#include <cstdint>
#include <string>
#include <map>

namespace S3
{
	class BinaryFile;

	typedef std::map<uint16_t, std::string> IDToNameMap;
	typedef std::map<std::string, uint16_t> NameToIDMap;

	class Level
	{
	public:
		explicit Level(const char* filename = nullptr);

		void open(const char* filename);
		void close();

	private:
		std::string m_name;

		IDToNameMap m_strings;

		IDToNameMap m_namesByIDs;
		NameToIDMap m_IDsByNames;

		NameToIDMap m_imageIDs;

		NameToIDMap m_soundIDs;
		std::map<std::string, uint8_t> m_soundGroups;
		std::map<uint8_t, float> m_soundGroupVolumes;

		void loadName(BinaryFile& fs);
		void loadSound(BinaryFile& fs);
		void loadAtlasTexture(BinaryFile& fs);
		void loadImageIDs(BinaryFile& fs);
		void loadStrings(BinaryFile& fs);
		void loadTimelime(BinaryFile& fs);
		void loadBuffers(BinaryFile& fs);
		void loadTexture(BinaryFile& fs);
	};

} // namespace S3
