#include "application/Application.h"
#include "application/TrivialLogger.h"

#include <fstream>

#include "easyexif/exif.h"

static std::vector<unsigned char> read(const std::string &file)
{
  using Bytes = std::vector<unsigned char>;
  std::error_code ec;
 
  std::ifstream in(file, std::ifstream::binary);
  if (!in)
    return Bytes{};


  in.unsetf(std::ios::skipws);

  in.seekg(0, std::ios::end);
  const auto size = in.tellg();
  in.seekg(0, std::ios::beg);

  std::vector<unsigned char> ret(static_cast<size_t>(size));
  in.read(reinterpret_cast<char*>(ret.data()), size);
  in.close();

  return ret;
}

// Visual ROW_COLUMN
enum class Orientation
{
  TOP_LEFT = 1,
  TOP_RIGHT = 2, 
  
  BOTTOM_RIGHT = 3,
  BOTTOM_LEFT = 4,

  LEFT_TOP = 5,
  RIGHT_TOP = 6,
  RIGHT_BOTTOM = 7,
  LEFT_BOTTOM = 8,

  UNDEFINED = 9 
};

enum class OrientationTransformation
{
  NONE = 1,

  FLIP_HORIZONTAL = 2,
  FLIP_VERTICAL = 4,

  ROT_180 = 3,
  ROT_90_CW = 6,
  ROT_270_CW = 8,
  
  FLIP_HORIZONTAL_AND_ROT_270_CW = 5,
  FLIP_HORIZONTAL_AND_ROT_90_CW = 7,

  UNDEFINED = 9
};

static const std::vector<std::string> orientTransfString = 
{
  {""},
  {"NONE"},
  {"FLIP_HORIZONTAL"},
  {"ROT_180"},
  {"FLIP_VERTICAL"},
  {"FLIP_HORIZONTAL_AND_ROT_270_CW"},
  {"ROT_90_CW"},
  {"FLIP_HORIZONTAL_AND_ROT_90_CW"},
  {"ROT_270_CW"},
  {"UNDEFINED"}
};

//std::string orientationAction(Orientation orient

class JpgRotate : public Application
{
public:
  JpgRotate(int argc, char *argv[]);

protected:
  int main() override;
};


JpgRotate::JpgRotate(int argc, char *argv[]):
  Application(argc, argv, "JpgRotate")
{ 
  Application::showHelpIfNoArguments(); 
  //Application::addCmdOption("option,o", "example of cmd option");
  //Application::addCmdOptionFlag("flag,f", "example of cmd flag");
  
  Application::addCmdOption("file,f", "Input file");
}

int JpgRotate::main()
{
  LOG_INF << "JpgRotate";

  const auto buf = read(Application::getCmdOptionValue("file"));
  if(buf.empty())
  {
    LOG_ERR << "Empty buf";
    return 0;
  }

  easyexif::EXIFInfo result;
  int code = result.parseFrom(buf.data(), static_cast<unsigned int>(buf.size()));
  
  if (code) {
    LOG_ERR << "Error parsing EXIF: code " << code;
    return 0;
  }

  const int orientCode = static_cast<int>( result.Orientation);
  const std::string transformationDescription = orientTransfString[orientCode];
  LOG_INF << "Orientation: " << orientCode << " " << transformationDescription;

  return 0;
}


int main(int argc, char *argv[])
{
  return MainApplication::run<JpgRotate>(argc, argv);
}
