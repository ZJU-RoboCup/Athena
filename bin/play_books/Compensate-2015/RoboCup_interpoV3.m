close all;
DISPLAY=false;

PRECISION_RES_SPEED=1;
PRECISION_RES_ANGLE=0.5;

%定义速度以及角度范围
RANGE_SPEED_LO=195;
RANGE_SPEED_HI=650;
RANGE_ANGLE_LO=0;
RANGE_ANGLE_HI=90;

%关键系数
for car=1:1:12
CURRENT_CAR_NUM=car;
COMPENSATION_GLOBAL=0.48;
 %                        全局   15°  25°  35°  45°   55°  65°  75° 85°    
COMPENSATION_DATA_GLOBAL=[0.48   -0.5   -1.0   2.5   5.0   5.0   5.0   5.0  5.0];

COMPENSATION_DATA=[
 COMPENSATION_DATA_GLOBAL
 COMPENSATION_DATA_GLOBAL
 COMPENSATION_DATA_GLOBAL
 COMPENSATION_DATA_GLOBAL
 COMPENSATION_DATA_GLOBAL
 COMPENSATION_DATA_GLOBAL
 COMPENSATION_DATA_GLOBAL
 COMPENSATION_DATA_GLOBAL
 COMPENSATION_DATA_GLOBAL
 COMPENSATION_DATA_GLOBAL
 COMPENSATION_DATA_GLOBAL
 COMPENSATION_DATA_GLOBAL
];
 %全局                        15°    25°    35°    45°    55°    65°    75°    85°    
 %{
 COMPENSATION_GLOBAL+0.0      0.0     0.0     0.0     0.0     0.0     0.0    0.0     0.0 %一号车
 COMPENSATION_GLOBAL+0.0      0.0     0.0     0.0     0.0     0.0     0.0    0.0     0.0 %二号车
 COMPENSATION_GLOBAL+0.0      0.0     0.0     4.0     4.0     4.0     4.0    4.0     4.0 %三号车
 COMPENSATION_GLOBAL+0.08     0.0     0.0     0.0     0.0     0.0     0.0    0.0     0.0 %四号车
 COMPENSATION_GLOBAL+0.0      0.0     0.0     0.0     0.0     0.0     0.0    0.0     0.0 %五号车
 COMPENSATION_GLOBAL+0.0      0.0     0.0     0.0     0.0     0.0     0.0    0.0     0.0 %六号车
 COMPENSATION_GLOBAL+0.0      0.0     0.0     0.0     0.0     0.0     0.0    0.0     0.0 %七号车
 COMPENSATION_GLOBAL+0.0      0.0     0.0     0.0     1.0     2.0     3.0    3.0     3.0 %八号车
 COMPENSATION_GLOBAL+0.0      0.0     0.0     0.0     0.0     0.0     0.0    0.0     0.0 %九号车
 COMPENSATION_GLOBAL+0.0      0.0     0.0     0.0     0.0     0.0     0.0    0.0     0.0 %十号车 
 COMPENSATION_GLOBAL+0.0      0.0     0.0     0.0     0.0     0.0     0.0    0.0     0.0 %十一号
 COMPENSATION_GLOBAL+0.0      0.0     0.0     0.0     0.0     0.0     0.0    0.0     0.0 %十二号
];%√
 %}
%2015/07/13第一次完整测试，by NaN

FIX_RATIO_GLOBAL=COMPENSATION_DATA(CURRENT_CAR_NUM,1); %总体修改
FIX_RATIO_15=COMPENSATION_DATA(CURRENT_CAR_NUM,2);        %具体角度修改
FIX_RATIO_25=COMPENSATION_DATA(CURRENT_CAR_NUM,3);
FIX_RATIO_35=COMPENSATION_DATA(CURRENT_CAR_NUM,4);
FIX_RATIO_45=COMPENSATION_DATA(CURRENT_CAR_NUM,5);
FIX_RATIO_55=COMPENSATION_DATA(CURRENT_CAR_NUM,6);
FIX_RATIO_65=COMPENSATION_DATA(CURRENT_CAR_NUM,7);
FIX_RATIO_75=COMPENSATION_DATA(CURRENT_CAR_NUM,8);
FIX_RATIO_85=COMPENSATION_DATA(CURRENT_CAR_NUM,9);

%-------------------------------------------------------------------------
%输入原始数据
matInput=importdata('input_20150708.txt');
SAMPLE_SPEED=1;
SAMPLE_ANGLE=2;
FIX_ANG=3;
Xi=matInput(:,SAMPLE_SPEED);
Yi=matInput(:,SAMPLE_ANGLE);
Zi=matInput(:,FIX_ANG);
XO=linspace(RANGE_SPEED_LO,RANGE_SPEED_HI,(RANGE_SPEED_HI-RANGE_SPEED_LO)/PRECISION_RES_SPEED);
YO=linspace(RANGE_ANGLE_LO,RANGE_ANGLE_HI,(RANGE_ANGLE_HI-RANGE_ANGLE_LO)/PRECISION_RES_ANGLE);
[Xo,Yo]=meshgrid(XO,YO);


%-------------------------------------------------------------------------
%数据过滤，删除偏差过大数据
SURF_DELTA=6.5; %偏差允许阈值 
Xtemp=[ones(length(Xi),1),Yi,Xi,Xi.*Yi,Xi.^2,Yi.^2]; 
[b]=regress(Zi,Xtemp); 
[m,~]=size(Xi);
count=1;
for i=1:1:m
    x=Xi(i);
    y=Yi(i);
    z=Zi(i);
    z_fit=b(1)+ y*b(2) + x*b(3)+x*y*b(4)+x^2*b(5)+y^2*b(6);
    z_hi=z_fit+SURF_DELTA;
    z_lo=z_fit-SURF_DELTA;
    if(z>=z_lo && z<=z_hi)
        Xi2(count)=x;
        Yi2(count)=y;
        Zi2(count)=z;
        count=count+1;
    end
end

%-------------------------------------------------------------------------
%过滤后进行滤波
Zo=griddata(Xi2,Yi2,Zi2,Xo,Yo,'cubic');
gausFilter1 = fspecial('gaussian',[14 5],7);
gausFilter2 = fspecial('average',12);
Zo_filter=imfilter(Zo,gausFilter1,'replicate');
Zo_filter=imfilter(Zo_filter,gausFilter2,'replicate');

%-------------------------------------------------------------------------
%精确拟合
Xtemp=[ones(length(Xi),1),Yi,(Xi-RANGE_SPEED_LO),(Xi-RANGE_SPEED_LO).*Yi,(Xi-RANGE_SPEED_LO).^2,Yi.^2]; %第二次拟合
[b]=regress(Zi,Xtemp);
Zo_fitted=b(1)+ Yo*b(2) + (Xo-RANGE_SPEED_LO)*b(3)+(Xo-RANGE_SPEED_LO).*Yo*b(4)+((Xo-RANGE_SPEED_LO).^2)*b(5)+(Yo.^2)*b(6);

[m,n]=size(Zo_fitted);%阈值化处理
for i=1:1:m
    for j=1:1:n
    if(Zo_fitted(i,j)<0)
        Zo_fitted(i,j)=0;
    end
    end
end

gausFilter1 = fspecial('gaussian',[30 15],7);%滤波，减少偏导突变
gausFilter2 = fspecial('average',15);
Zo_fitted=imfilter(Zo_fitted,gausFilter1,'replicate');
Zo_fitted=imfilter(Zo_fitted,gausFilter2,'replicate');

Zo_fitted=Zo_fitted*FIX_RATIO_GLOBAL;%全局修正比例

%-------------------------------------------------------------------------
%特定角度段修正

%FILTER_X=2;
%FILTER_Y=2;
%FILTER_C1=1;
%FILTER_C2=1;
%Filter1 = fspecial('gaussian',[FILTER_X FILTER_Y],FILTER_C1);%滤波，减少偏导突变
%Filter2 = fspecial('average',FILTER_C2);
%fix_mat_15=imfilter(fix_mat_15,Filter1,'replicate');
%fix_mat_15=imfilter(fix_mat_15,Filter2,'replicate');

TRAP_A=7;
TRAP_B=6;

%15度补偿图
fix_mat_15=zeros(m,n);
mid=15/PRECISION_RES_ANGLE;
trap=zeros(m,1);
for i=mid-TRAP_A:1:mid+TRAP_A
    trap(i)=1;
end
for i=mid-TRAP_A-TRAP_B:1:mid-TRAP_A
    trap(i)=(i-(mid-TRAP_A-TRAP_B))/TRAP_B;
end
for i=mid+TRAP_A:1:mid+TRAP_A+TRAP_B
    trap(i)=((mid+TRAP_A+TRAP_B)-i)/TRAP_B;
end
for i=1:1:m
    fix_mat_15(i,:)=linspace(0,trap(i),n);
end

%25度补偿图
fix_mat_25=zeros(m,n);
mid=25/PRECISION_RES_ANGLE;
trap=zeros(m,1);
for i=mid-TRAP_A:1:mid+TRAP_A
    trap(i)=1;
end
for i=mid-TRAP_A-TRAP_B:1:mid-TRAP_A
    trap(i)=(i-(mid-TRAP_A-TRAP_B))/TRAP_B;
end
for i=mid+TRAP_A:1:mid+TRAP_A+TRAP_B
    trap(i)=((mid+TRAP_A+TRAP_B)-i)/TRAP_B;
end
for i=1:1:m
    fix_mat_25(i,:)=linspace(0,trap(i),n);
end

%35度补偿图
fix_mat_35=zeros(m,n);
mid=35/PRECISION_RES_ANGLE;
trap=zeros(m,1);
for i=mid-TRAP_A:1:mid+TRAP_A
    trap(i)=1;
end
for i=mid-TRAP_A-TRAP_B:1:mid-TRAP_A
    trap(i)=(i-(mid-TRAP_A-TRAP_B))/TRAP_B;
end
for i=mid+TRAP_A:1:mid+TRAP_A+TRAP_B
    trap(i)=((mid+TRAP_A+TRAP_B)-i)/TRAP_B;
end
for i=1:1:m
    fix_mat_35(i,:)=linspace(0,trap(i),n);
end

%45度补偿图
fix_mat_45=zeros(m,n);
mid=45/PRECISION_RES_ANGLE;
trap=zeros(m,1);
for i=mid-TRAP_A:1:mid+TRAP_A
    trap(i)=1;
end
for i=mid-TRAP_A-TRAP_B:1:mid-TRAP_A
    trap(i)=(i-(mid-TRAP_A-TRAP_B))/TRAP_B;
end
for i=mid+TRAP_A:1:mid+TRAP_A+TRAP_B
    trap(i)=((mid+TRAP_A+TRAP_B)-i)/TRAP_B;
end
for i=1:1:m
    fix_mat_45(i,:)=linspace(0,trap(i),n);
end

%55度补偿图
fix_mat_55=zeros(m,n);
mid=55/PRECISION_RES_ANGLE;
trap=zeros(m,1);
for i=mid-TRAP_A:1:mid+TRAP_A
    trap(i)=1;
end
for i=mid-TRAP_A-TRAP_B:1:mid-TRAP_A
    trap(i)=(i-(mid-TRAP_A-TRAP_B))/TRAP_B;
end
for i=mid+TRAP_A:1:mid+TRAP_A+TRAP_B
    trap(i)=((mid+TRAP_A+TRAP_B)-i)/TRAP_B;
end
for i=1:1:m
    fix_mat_55(i,:)=linspace(0,trap(i),n);
end

%65度补偿图
fix_mat_65=zeros(m,n);
mid=65/PRECISION_RES_ANGLE;
trap=zeros(m,1);
for i=mid-TRAP_A:1:mid+TRAP_A
    trap(i)=1;
end
for i=mid-TRAP_A-TRAP_B:1:mid-TRAP_A
    trap(i)=(i-(mid-TRAP_A-TRAP_B))/TRAP_B;
end
for i=mid+TRAP_A:1:mid+TRAP_A+TRAP_B
    trap(i)=((mid+TRAP_A+TRAP_B)-i)/TRAP_B;
end
for i=1:1:m
    fix_mat_65(i,:)=linspace(0,trap(i),n);
end

%75度补偿图
fix_mat_75=zeros(m,n);
mid=75/PRECISION_RES_ANGLE;
trap=zeros(m,1);
for i=mid-TRAP_A:1:mid+TRAP_A
    trap(i)=1;
end
for i=mid-TRAP_A-TRAP_B:1:mid-TRAP_A
    trap(i)=(i-(mid-TRAP_A-TRAP_B))/TRAP_B;
end
for i=mid+TRAP_A:1:mid+TRAP_A+TRAP_B
    trap(i)=((mid+TRAP_A+TRAP_B)-i)/TRAP_B;
end
for i=1:1:m
    fix_mat_75(i,:)=linspace(0,trap(i),n);
end

%85度补偿图
fix_mat_85=zeros(m,n);
mid=85/PRECISION_RES_ANGLE;
trap=zeros(m,1);
for i=mid-TRAP_A:1:mid+TRAP_A
    trap(i)=1;
end
for i=mid-TRAP_A-TRAP_B:1:mid-TRAP_A
    trap(i)=(i-(mid-TRAP_A-TRAP_B))/TRAP_B;
end
for i=mid+TRAP_A:1:mid+TRAP_A+TRAP_B
    trap(i)=((mid+TRAP_A+TRAP_B)-i)/TRAP_B;
end
for i=1:1:m
    fix_mat_85(i,:)=linspace(0,trap(i),n);
end

fix_mat_tot=fix_mat_15+fix_mat_25+fix_mat_35+fix_mat_45+fix_mat_55+fix_mat_65+fix_mat_75+fix_mat_85;

Zo_final=Zo_fitted+FIX_RATIO_15*fix_mat_15;
Zo_final=Zo_final+FIX_RATIO_25*fix_mat_25;
Zo_final=Zo_final+FIX_RATIO_35*fix_mat_35;
Zo_final=Zo_final+FIX_RATIO_45*fix_mat_45;
Zo_final=Zo_final+FIX_RATIO_55*fix_mat_55;
Zo_final=Zo_final+FIX_RATIO_65*fix_mat_65;
Zo_final=Zo_final+FIX_RATIO_75*fix_mat_75;
Zo_final=Zo_final+FIX_RATIO_85*fix_mat_85;

gausFilter1 = fspecial('gaussian',[5 5],3);
gausFilter2 = fspecial('average',3);
Zo_final=imfilter(Zo_final,gausFilter1,'replicate');
Zo_final=imfilter(Zo_final,gausFilter2,'replicate');
%-------------------------------------------------------------------------
%Display

if(DISPLAY==true)
    
[m1,n1]=size(Zi);
Size1=ones(m1,1)*12;
Color1=[ones(m1,1)*1.0,ones(m1,1)*0.0,ones(m1,1)*0.0];
[m2,n2]=size(Zi2);
Size2=ones(m2,1)*12;
Color2=[ones(m2,1)*0.0,ones(m2,1)*1.0,ones(m2,1)*0.3];


figure(car);
hold on;
scatter3(Xi,Yi,Zi*FIX_RATIO_GLOBAL,Size1,Color1);
scatter3(Xi2,Yi2,Zi2*FIX_RATIO_GLOBAL,Size2,Color2);
mesh(Xo,Yo,Zo_final);
hold off;
axis([RANGE_SPEED_LO,RANGE_SPEED_HI,RANGE_ANGLE_LO,RANGE_ANGLE_HI]);
xlabel('Speed'),ylabel('Angle'),zlabel('FixAngle');
end

%-------------------------------------------------------------------------
%输出
X_OUT_MAX=650;
X_OUT_MIN=195;
Y_OUT_MAX=80;
Y_OUT_MIN=0;
X_COUNT=(X_OUT_MAX-X_OUT_MIN)/5+1;
Y_COUNT=(Y_OUT_MAX-Y_OUT_MIN)/5+1;
Xfix=X_OUT_MIN:5:X_OUT_MAX;
Yfix=Y_OUT_MIN:5:Y_OUT_MAX;
[Xf,Yf]=meshgrid(Xfix,Yfix);
Zf=interp2(Xo,Yo,Zo_final,Xf,Yf);
Zf(1,:)=Zf(1,:)*0;
Zf=Zf';

strNum=num2str(CURRENT_CAR_NUM);
strName='data-';
strFileNameXls=[strName,strNum,'.xlsx'];
strFileNameTxt=[strName,strNum,'.txt'];

%输出txt
fout=fopen(strFileNameTxt,'wt');
[m,n]=size(Zf);
fprintf(fout,'%d %d\n',m,n);

for i=1:1:m
    for j=1:1:n
        if j==n
            fprintf(fout,'%.6f\n',Zf(i,j));
        else 
            fprintf(fout,'%.6f\t',Zf(i,j));
        end
    end
end
fclose(fout);

%输出xls

%%[stat,msg]=xlswrite(strFileNameXls, Zf, 'Compensation_Result');
end
finish='all done'
