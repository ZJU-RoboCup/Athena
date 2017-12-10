这个目录存放程序的参数,用标准XML文件格式存放参数,举例如下：
<?xml version="1.0" encoding="UTF-8"?>
<ProjectName>
	<ClassName1>
		<ParamName>ParamValue</ParamName>
		<ParamName2>ParamValue2</ParamName2>
	</ClassName1>
	<ClassName2>
		<ParamName3>ParamValue3</ParamName3>
		<ParamName4>ParamValue4</ParamName4>
	</ClassName2>
</ProjectName>
其中bool型参数用 1 表示 true, 0 表示 false.
SmallPlayer.xml 是 SmallPlayer.exe 用到的参数文件
SmallSim.xml 是 SmallSim.exe 用到的参数文件
SmallPlayer_dist.xml 是 SmallPlayer.exe 缺省的参数, 由程序自动生成
SmallSim_dist.xml 是 SmallSim.exe 缺省的参数，由程序自动生成