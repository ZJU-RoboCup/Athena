v_in = data(1:2:length(data(:, 1)), 1)';
theta_in = data(1:2:length(data(:, 1)), 2)';
out = 500 * data(1:2:length(data(:, 1)), 3)';

%--------------------------

[input,ps1]=mapminmax([v_in;theta_in]);%两行十列
[target,ps2]=mapminmax([out]); %一行十列

net=newff(input,target,6,{'tansig','purelin'},'trainlm');
net.trainParam.epochs=1000;
net.trainParam.goal=0.0000001;%目标最小误差
LP.lr=0.000001;%学习速率
net=train(net,input,target);

test_v = data(2:2:length(data(:, 1)), 1)';
test_theta = data(2:2:length(data(:, 1)), 2)';
test_out = 500 * data(2:2:length(data(:, 1)), 3)';

input1=mapminmax('apply',[test_v;test_theta],ps1);
output1=net(input1);
prediction1=mapminmax('reverse',output1,ps2);
figure;
plot(test_out,'*','color',[222 87 18]/255);hold on
plot(prediction1,'-o','color',[244 208 0]/255,...
'linewidth',2,'MarkerSize',14,'MarkerEdgecolor',[138 151 123]/255);



