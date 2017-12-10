function reMmodel = newfitRANSAC(x, y)
    out = y;
    v_in = x(:, 1);
    theta_in = x(:, 2);
    sigma = 0.5;
% v_out * sin(theta_out) = A * v_in * sin(theta_in) + B + C * v_in^2;    

model = zeros(1, 3);
resModel = zeros(1, 3);
maxInterNum = 0;

t = sqrt(3.84 * sigma^2);

%sample param
N = length(x);
numSample = N * log(N); iter = 0; s = 2; epsilon = 0.5; p = 0.99;


while iter < numSample
    sample1 = floor(N * rand) + 1;
    sample2 = floor(N * rand) + 1;
    sample3 = floor(N * rand) + 1;
    while sample1 == sample2
        sample2 = floor(N * rand) + 1;
    end
    while sample3 == sample1 || sample3 == sample2
        sample3 = floor(N * rand) + 1;
    end
    model = getFitParam([x(sample1, :), y(sample1)], [x(sample2, :), y(sample2)], [x(sample3, :), y(sample3)]);
    interPointNum = getInterPointNum(x, y, model, t);
    if interPointNum > maxInterNum
        maxInterNum = interPointNum;
        resModel = model;
    end
    
    epsilon = 1 - maxInterNum/N;
    % numSample = 2 * log(1-p)/log(1-(1-epsilon)^s);
    iter = iter + 1;
end

figure, scatter(x(:, 1) .^ sin(x(:, 2)), y)
v_in_ = 0 : 0.5 : max(v_in);
theta_in_ = 0 : max(theta_in)/30: max(theta_in);
xx = v_in_' * sin(theta_in_);
yy = resModel(1) * xx + resModel(2) + resModel(3) * v_in_'.^2  *ones(size(theta_in_));
yy = reshape(yy, [1, size(yy, 1) * size(yy, 2)]);
xx = reshape(xx, [1, size(xx, 1) * size(xx, 2)]);
%yy = resModel(1) * v_in_ * sin(theta_in_) + resModel(2) + resModel(3) * v_in_ .^2;
hold on
scatter(xx, yy)
%plot(xx, yy+t, '-g', xx, yy - t, '-g')

%% Robust cost function
%robustCostModel = fmincon(@(resModel)robustCosrFun(x, y, t, resModel), resModel,[], [])
%yy = robustCostModel(1) * xx + robustCostModel(2);
%plot(xx, yy, 'b')

    
    
end


function model = getFitParam(x1, x2, x3)
    % 3 params: 3 points
    v_in1 = x1(1);
    v_in2 = x2(1);
    v_in3 = x3(1);
    theta_in1 = x1(2);
    theta_in2 = x2(2);
    theta_in3 = x3(2);
    A = [v_in1*sin(theta_in1), 1, v_in1^2;
         v_in2*sin(theta_in2), 1, v_in2^2;
         v_in3*sin(theta_in3), 1, v_in3^2;];
     b = [x1(3), x2(3), x3(3)]';
     model = A\b;
end

function num = getInterPointNum(x, y, model, t)
    num = 0;
    for i = 1 : length(x)
        if (y(i) - (model(1) * x(i, 1) * sin(x(i, 2)) + model(2) + model(3) * x(i, 1)^2)) ^ 2 <= t^2
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

