/// FileName : 		WirelessModule.h
/// 				declaration file
/// Description :	It supports command send interface for ZJUNlict,
///	Keywords :		fitting, send, interface
/// Organization : 	ZJUNlict@Small Size League
/// Author : 		cliffyin
/// E-mail : 		cliffyin@zju.edu.cn
///					cliffyin007@gmail.com
/// Create Date : 	2011-07-25
/// Modified Date :	2011-07-25 
/// History :

#ifndef _WIRELESS_MODULE_H_
#define _WIRELESS_MODULE_H_

#include "CommControl.h"
#include "RobotCommand.h"

/// <summary>	Wireless module.  </summary>
///
/// <remarks>	ZjuNlict, 2011-7-26. </remarks>

class CWirelessModule
{
public:

	/// <summary>	Default constructor. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-26. </remarks>

	CWirelessModule(void);

	/// <summary>	Finaliser. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-26. </remarks>

	~CWirelessModule(void);

	/// <summary>	Robot Command Send Interface. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-26. </remarks>
	///
	/// <param name="index">	   	Real robot index. </param>
	/// <param name="robotCommand">	The robot command. </param>

	void doWirelessModule(int index, const ROBOTCOMMAND& robotCommand);

	/// <summary>	Gets the control communications. </summary>
	///
	/// <remarks>	ZjuNlict, 2011-7-26. </remarks>
	///
	/// <returns>	null if it fails, else the control communications. </returns>

	CCommControl* getControlComm() const;

private:

	/// <summary> The communications control </summary>
	CCommControl *pCommControl;
};

#endif