clear;

Pxux = zeros(3,3,3);
Pzx = zeros(3,3);
Init_bel = zeros(1,3);

Attack = 1;
Deadlock = 2;
Defend = 3;

outputfilename = 'output.txt';
[cycle,Out_u,Out_z,Out_bel,bayesfilename] = ReadOutputFile(outputfilename);
[Pxux,Pzx,Init_bel]= ReadBayesFile(bayesfilename);

sbel = zeros(size(Out_bel,1)+1,size(Out_bel,2));

bel = Init_bel;
bel_bar = bel;
sbel(1,:) = bel; 
for iter_cycle = 1: length(cycle)
    prebel = zeros(1,3);
    u = Out_u(iter_cycle);
    z = Out_z(iter_cycle);
    for iter = 1:3
        for lastiter = 1:3
            prebel(iter) = prebel(iter) + Pxux(lastiter,u,iter)*bel_bar(lastiter);
        end
    end
    for iter = 1:3
        bel(iter) = Pzx(z,iter)*prebel(iter);
    end
    bel = bel./sum(bel);
    bel_bar = bel;
    
    sbel(iter_cycle+1,:) = bel;
end

% [state_val, state]=max(bel);
% if 1 == state
%     disp('Attack');
% elseif 2 == state
%     disp('Deadlcok');
% elseif 3 == state
%     disp('Defend');
% end
