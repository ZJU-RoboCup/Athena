function err = MyFit(v_in, theta_in, theta_out, x)
    A = x(1);
    %B = x(2);
    %C = x(3);
    D = x(2);
    % E = x(5);
    err = 0;
    speed = 500;
    for i = 1 : length(v_in)
       fit = A * v_in(i) * sin(theta_in(i)) +D ;%+ B * v_in(i) + C * sin(theta_in(i))...
          % + D;
       err = err + (fit - speed * sin(theta_out(i)))^2;
    end
    err = err / length(v_in);
end