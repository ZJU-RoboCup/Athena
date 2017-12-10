#ifndef _FILE_PARAM_H_
#define _FILE_PARAM_H_
#include <string>
namespace Param{
	namespace File{
		const std::string DataDir = "data\\";
		const char* const ParamDir = "params\\";
		const std::string RobotPosFilterDir = "vision\\Robot_Param\\Pos\\";
		const std::string RobotRotFilterDir = "vision\\Robot_Param\\Rotation\\";
		const std::string BallFilterDir = "vision\\Ball_Param\\";
	}
	namespace Output{
		const std::string NewLineCharacter = "\r\n";
	}
}
#endif // _FILE_PARAM_H_
