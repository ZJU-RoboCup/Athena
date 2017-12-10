clear();
[xR, yR] = textread('carVisionVel.txt', '%f %f');
[x, y] = textread('carControlCommand.txt', '%f %f');
plot(xR);
hold on;
plot(yR);
plot(x);
plot(y);