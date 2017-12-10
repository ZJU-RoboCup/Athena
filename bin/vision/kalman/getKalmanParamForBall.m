close all
slowResponse = 20;
fastResponse = 2000;
Pos = dlmread('./data/ball/Test1.txt');

Tsample = 1/60;
isDraw = 1;
titleStr = 'slow-ball-x-';
filtDesignVel(Pos(:,2),Tsample,slowResponse, isDraw, titleStr);
titleStr = 'slow-ball-y-';
filtDesignVel(Pos(:,3),Tsample,slowResponse, isDraw, titleStr);
titleStr = 'fast-ball-x-';
filtDesignVel(Pos(:,2),Tsample,fastResponse, isDraw, titleStr);
titleStr = 'fast-ball-y-';
filtDesignVel(Pos(:,3),Tsample,fastResponse, isDraw, titleStr);