clear;

Pxux = zeros(3,3,3);
Pzx = zeros(3,3);
Init_bel = zeros(1,3);

Attack = 1;
Deadlock = 2;
Defend = 3;

filename = 'Skuba_Attack.bayes';
[Pxux,Pzx,Init_bel]= ReadBayesFile(filename);

bel = Init_bel;
bel_bar = bel;

prebel = zeros(1,3);
u = Attack;
z = Attack;
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
[state_val, state]=max(bel);
if 1 == state
    disp('Attack');
elseif 2 == state
    disp('Deadlcok');
elseif 3 == state
    disp('Defend');
end
