功能描述
	GetFilesMD5.exe 计算所在目录的各个文件内容的MD5值,并记录MD5值到UpdateFileList.ver,(json格式).
	完成后,屏幕会打印各个文件内容的MD5值相加字符串的MD5值,你可以复制这个值到UpdateTotal.ver里

命名参数
	-url	例如 -url test.update.com/
				程序会插入一段json到文件 "url":"test.update.com/"
	
	-nodir 例如 -nodir tmpdir	
				程序会跳过tmpdir目录
				例如2 -nodir tmpdir/test
				程序会跳过 tmpdir目录下的test目录
	
	-nofile 例如 -nofile exe ver
				程序会跳过以.exe .ver结尾的文件
	
	-dir	例如 -dir jamj
				程序在记录UpdateFileList.ver时,会给所有文件增加一个jamj的目录
	
	-help	程序会打印所有的命令以及用法
				不能带其他的命令和参数
	
	参见GetFilesMD5.bat