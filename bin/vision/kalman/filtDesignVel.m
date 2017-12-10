 function ballEstimated=filtDesignVel(Pos,Tsample,speedResponse, isDraw, titleStr) 
 %
 % Filter Design for Ball Position and Velocity Estimation
 % Can also be used for Opponent Robot Position and Velocity Estimation.
 %
 % Each axis, or degree of freedom, is considered independently.
 %


 %            Pos            n * 1          data to be filtered
 %            Tsample        1 * 1          sampling time
 %            speedResponse  1*1            tradeoff between filtering smoothness and how
 %                                          quickly we want the filter to react.  
   
 %                                          Large number means fast response, but noisy;
 % if it is too noisy, we will get erratic estimations of the ball position far into
 % the future, because there will be large uncertainty in the velocity estimation.  If
 % it is too slow, we will be slow to react to sudden changes in velocity, such as hitting
 % a wall or robot.  
 
 % State Equations; captures the dynamics of the ball.  Assumes that there is no
 % friction, this is OK for filtering purposes.  Sets equations up for KALMAN command.
 
 A=[0 1;0 0];
 B=zeros(2,0);  % no control input
 
 G=[0;1];
 C=[1 0];
 D=zeros(1,0);  % no control input
 H=0;
 
 QN=speedResponse^2;
 RN=1;
 NN=0;
 
 sys=ss(A,[B G],C,[D H]);
 [KEST,L,P] = kalman(sys,QN,RN,NN);

 % We have just derived the continuous time filter.  The input to the filter is the 
 % measurement.  The first output of the filter is the estimated 
 % position;  the second output is the estimated
 % velocity.
 
 Afilt=A-L*C;
 Bfilt=L;
 Cfilt=eye(2);
 Dfilt=zeros(2,1);
% $$$  Afilt;
% $$$  Bfilt;
% $$$  Cfilt;
% $$$  Dfilt;                                 
 sysFilt=ss(Afilt,Bfilt,Cfilt,Dfilt);
 
 %
 % Analyze the performance of the system.  Try the filter out on some test data.
 %
 length = size(Pos);
 t=0:Tsample:(length(1)-1)*Tsample; 
  % add noise to the measurement
 ballEstimated=lsim(sysFilt,Pos,t);
 
 if 1 == isDraw
 figure
    plot(t,Pos,'.',t,ballEstimated(:,1),'-g.',t,ballEstimated(:,2),'r'); 
    grid
    legend('measured','filtered pos','filtered vel')
    title([titleStr,'-qr = ', num2str(speedResponse)] );
 end
 sysFiltImplement = c2d(sysFilt,Tsample,'foh');
 % Simulate again, should get very similar results.  Since we created the data
 % using our sampling rate, we can just do direct discrete time simulation.
 ballEstimated=lsim(sysFiltImplement,Pos);
% $$$  figure(2)
% $$$  plot(t,Pos,'.',t,ballEstimated(:,1),t,ballEstimated(:,2)); grid
% $$$  legend('measured','filtered pos','filtered vel')
% $$$  
 [Aimp,Bimp,Cimp,Dimp]=ssdata(sysFiltImplement);
 lossVec = (eye(2)-Aimp)^(-1)*Bimp;
 
 %test cornel c
 %  先做一个简单的，不考虑两个分辨率滤波的比较
 
 
 % output the matrix to a file
 
 fid = fopen(['./param/Matrices-',num2str(speedResponse),'.txt'],'w');
 fprintf(fid,'%f %f\r\n%f %f\r\n',Aimp(1,1),Aimp(1,2),Aimp(2,1),Aimp(2,2));
 fprintf(fid,'%f %f\r\n',Bimp(1),Bimp(2));
 fprintf(fid,'%f %f\r\n%f %f\r\n',Cimp(1,1),Cimp(1,2),Cimp(2,1),Cimp(2,2));
 fprintf(fid,'%f %f\r\n',Dimp(1),Dimp(2));
 fprintf(fid,'%f %f\r\n',lossVec(1),lossVec(2));
 fclose(fid);
 
 % The estimate of the current velocity and position of the ball,
 % in the camera frame of reference, can thus be calculated as follows:
 %
 % filtOut = Cimp*filtState + Dimp*ballPosMeasure
 % filtState = Aimp*filtState + Bimp*ballPosMeasure
 %
 % ballPosEstimate = filtOut(1);
 % ballVelEstimate = filtOut(2);
 %
 % You will then need to "push" forward the measurements, using the estimated
 % velocity, by the latency number of frames.  The simplest way to do this is
 %
 % ballPosEstimateRobotFrame = ballPosEstimate + ballVelEstimate*Tsample*FramesLatency
 %
 %
 % The above only applies to one degree of freedom (say the x-measurement of the ball).
 % A similar filter has to be applied for the y-position.
 %
 % The exact same filters can be applied to estimate the opponent robot position
 % and velocity. 
end