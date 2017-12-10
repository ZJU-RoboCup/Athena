#include "OptionModule.h"
#include <param.h>
#include <weerror.h>
#include "WorldModel.h"

namespace{
	int DEFAULT_NUMBER = 1;
	int DEFAULT_TYPE = 2;
	int DEFAULT_COLOR = TEAM_YELLOW;
}
COptionModule::COptionModule(int argc,char* argv[])
: _side(Param::Field::POS_SIDE_LEFT)
{
	_color = DEFAULT_COLOR;
	_number = DEFAULT_NUMBER;
	_type = DEFAULT_TYPE;
	ReadOption(argc, argv);
	std::cout << "Side : " << ((_side == Param::Field::POS_SIDE_LEFT) ? "left" : "right")
			  << ", Color : " << ((_color == TEAM_YELLOW) ? "yellow" : "blue") << " is running..." << Param::Output::NewLineCharacter;
	WorldModel::Instance()->registerOption(this);
}

COptionModule::~COptionModule(void)
{

}

void COptionModule::ReadOption(int argc, char* argv[])
{
	for( int i=1; i<argc; ++i ){
		switch( argv[i][0] ){
		case 's':
			// side
			if( argv[i][1] == 'l' ){
				_side = Param::Field::POS_SIDE_LEFT;
			}else if( argv[i][1] == 'r' ){
				_side = Param::Field::POS_SIDE_RIGHT;
			}else{
				throw WEError(__FILE__,__LINE__,"Side must be l or r");
			}
			break;
		case 'c':
			// color
			if( argv[i][1] == 'y' ){
				_color = TEAM_YELLOW;
			}else if( argv[i][1] == 'b' ){
				_color = TEAM_BLUE;
			}else{
				throw WEError(__FILE__,__LINE__,"Color must be y or b");
			}
			break;
		default:
			break;
		}
	}
}

