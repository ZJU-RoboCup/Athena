%--------------------------------------------------------------------------
% New Compensation Method.
%--------------------------------------------------------------------------
% INPUT FORMAT EXAMPLE : 
%   [ INPUT_SPEED , INPUT_ANGEL , sin(OUTPUT_ANGEL)*OUTPUT_SPEED ]
%--------------------------------------------------------------------------

ratio = 1;

SHOOT_SPEED = 650;
% INPUT_FILE = '7.262.txt';
% data = importdata(INPUT_FILE);
in_speed = data(:,1);
in_angel = data(:,2);
out_speed = 500;
out_angel = data(:,4);
in = in_speed.*sin(in_angel);
out = out_speed.*sin(out_angel);

x = in_speed;
y = in_angel;
z = out;

p = polyfit(in,out,1)
p2 = fitRANSAC(in, out)
A = p(1);
B = p(2);

X_OUT_MAX=650;
X_OUT_MIN=195;
Y_OUT_MAX=80;
Y_OUT_MIN=0;
X_COUNT=(X_OUT_MAX-X_OUT_MIN)/5+1;
Y_COUNT=(Y_OUT_MAX-Y_OUT_MIN)/5+1;
Xfix=X_OUT_MIN:5:X_OUT_MAX;
Yfix=Y_OUT_MIN:5:Y_OUT_MAX;
[Speed,Angel]=meshgrid(Xfix,Yfix);
%Z = A*Speed.*sin(Angel/180*pi)+B;
Z = A*Speed.*sin(Angel/180*pi);
%Z(1,:) = Z(1,:)*-1*0;
Z = atan(Z/SHOOT_SPEED)*180/pi * ratio;
dlmwrite('output.txt',Z','\t');