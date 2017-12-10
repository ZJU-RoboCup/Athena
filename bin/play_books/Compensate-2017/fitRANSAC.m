function resModel = fitRANSAC(x, y)
    sigma = 30;
    
model = zeros(1, 2);
resModel = zeros(1, 2);
maxInterNum = 0;

t = sqrt(3.84 * sigma^2);

%sample param
N = length(x);
numSample = floor(N*log(N)); iter = 0; s = 2; epsilon = 0.5; p = 0.99;


while iter < numSample
    sample1 = floor(N * rand) + 1;
    sample2 = floor(N * rand) + 1;
    while sample1 == sample2
        sample2 = floor(N * rand) + 1;
    end
    model = getLinearParam([x(sample1), y(sample1)], [x(sample2), y(sample2)]);
    interPointNum = getInterPointNum(x, y, model, t);
    if interPointNum > maxInterNum
        maxInterNum = interPointNum;
        resModel = model;
    end
    
    epsilon = 1 - maxInterNum/N;
    %numSample = 3 * log(1-p)/log(1-(1-epsilon)^s);
    iter = iter + 1;
end

figure, scatter(x, y)
xx = 0 :0.5 : max(x);
yy = resModel(1) * xx + resModel(2);
hold on
plot(xx, yy)
plot(xx, yy+t, '-g', xx, yy - t, '-g')

%% Robust cost function
%robustCostModel = fmincon(@(resModel)robustCosrFun(x, y, t, resModel), resModel,[], [])
%yy = robustCostModel(1) * xx + robustCostModel(2);
%plot(xx, yy, 'b')

    
    
end


function model = getLinearParam(x1, x2)
    % a line determined by point x1 and x2
    model(1) = (x1(2) - x2(2))/(x1(1) - x2(1));
    model(2) = (x2(1) * x1(2) - x1(1) * x2(2)) / (x2(1) - x1(1))^2;
end

function num = getInterPointNum(x, y, model, t)
    num = 0;
    for i = 1 : length(x)
        if (y(i) - (model(1) * x(i) + model(2)))^2 <= t^2
        num = num + 1;
        end
    end
end

function cost = robustCosrFun(x, y, t, resModel)
    cost = 0;
    for i = 1 : length(x)
        temp = (y(i) - (resModel(1) * x(i) + resModel(2)))^2;
        if temp > t ^2
            cost = cost + t^2;
        else
            cost = cost + temp^2;
        end
    end
    cost
end

