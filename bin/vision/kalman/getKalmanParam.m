close all
speedResponse =20;
Tsample = 1/60;
start = 1;
final = 30;
Pos = 0:Tsample:10;
Pos = Pos'+ rand(size(Pos,2), size(Pos,1))/10;
isDraw = 1;
titleStr = 'test';
ballEstimatedx=filtDesignVel(Pos,Tsample,speedResponse, isDraw, titleStr);
