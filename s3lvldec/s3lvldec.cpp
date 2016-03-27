
#include <cstdlib>
#include <cstdio>
#include <string>


namespace
{

class LevelData
{
public:
	explicit LevelData(const char* filename);
	~LevelData();

	const std::string& error() const { return m_error; }

private:
	std::string m_error;

};

LevelData::LevelData(const char* filename)
{

}

LevelData::~LevelData()
{

}

} // unnamed namespace


int main(int argc, char** argv)
{
	if (2 != argc)
	{
		printf("Usage: %s file.lvl\n", argv[0]);
		return EXIT_SUCCESS;
	}

	LevelData level(argv[1]);
	const std::string& error = level.error();

	if (error.empty())
	{
		// TODO
	}
	else
	{
		printf("ERROR: %s\n", error.c_str());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
