PRECISION_RES_SPEED=1;
PRECISION_RES_ANGLE=0.5;

%定义速度以及角度范围
RANGE_SPEED_LO=195;
RANGE_SPEED_HI=650;
RANGE_ANGLE_LO=0;
RANGE_ANGLE_HI=90;

%关键系数
CURRENT_CAR_NUM=8;
COMPENSATION_DATA=[
%全局      15°    25°    35°    45°    55°    65°    75°    85°    
 0.52     0.0    -0.3    -1.2    -1.2    -0.3    0.5     0.8     1.8 %一号车
 0.52     0.0    -0.3    -1.2    -1.2    -0.3    0.5     0.8     1.8 %二号车
 0.52     0.0    -0.3    -1.2    -1.2    -0.3    0.5     0.8     1.8 %三号车
 0.52     0.0    -0.3    -1.2    -1.2    -0.3    0.5     0.8     1.8 %四号车
 0.52     0.0    -0.3    -1.2    -1.2    -0.3    0.5     0.8     1.8 %五号车
 0.52     0.0    -0.3    -1.2    -1.2    -0.3    0.5     0.8     1.8 %六号车
 0.52     0.0    -0.3    -1.2    -1.2    -0.3    0.5     0.8     1.8 %七号车
 0.52     0.0    -0.3    -1.2    -1.2    -0.3    0.5     0.6     1.1 %八号车
 0.52     0.0    -0.3    -1.2    -1.2    -0.3    0.5     0.8     1.8 %九号车
 0.52     0.0    -0.3    -1.2    -1.2    -0.3    0.5     0.8     1.8 %十号车
 0.52     0.0    -0.3    -1.2    -1.2    -0.3    0.5     0.8     1.8 %十一号车
 0.52     0.0    -0.3    -1.2    -1.2    -0.3    0.5     0.8     1.8 %十二号车
];

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
RANGE=10;
FILTER_X=12;
FILTER_Y=12;
FILTER_C1=7;
FILTER_C2=8;

FIX_LO=0.1;

Filter1 = fspecial('gaussian',[FILTER_X FILTER_Y],FILTER_C1);%滤波，减少偏导突变
Filter2 = fspecial('average',FILTER_C2);

fix_mat_15=zeros(m,n);
for i=-RANGE:1:RANGE
    fix_mat_15(15/PRECISION_RES_ANGLE+i,:)=linspace(FIX_LO,1,n);
end
fix_mat_15=imfilter(fix_mat_15,Filter1,'replicate');
fix_mat_15=imfilter(fix_mat_15,Filter2,'replicate');

fix_mat_25=zeros(m,n);
for i=-RANGE:1:RANGE
    fix_mat_25(25/PRECISION_RES_ANGLE+i,:)=linspace(FIX_LO,1,n);
end
fix_mat_25=imfilter(fix_mat_25,Filter1,'replicate');
fix_mat_25=imfilter(fix_mat_25,Filter2,'replicate');

fix_mat_35=zeros(m,n);
for i=-RANGE:1:RANGE
    fix_mat_35(35/PRECISION_RES_ANGLE+i,:)=linspace(FIX_LO,1,n);
end
fix_mat_35=imfilter(fix_mat_35,Filter1,'replicate');
fix_mat_35=imfilter(fix_mat_35,Filter2,'replicate');

fix_mat_45=zeros(m,n);
for i=-RANGE:1:RANGE
    fix_mat_45(45/PRECISION_RES_ANGLE+i,:)=linspace(FIX_LO,1,n);
end
fix_mat_45=imfilter(fix_mat_45,Filter1,'replicate');
fix_mat_45=imfilter(fix_mat_45,Filter2,'replicate');

fix_mat_55=zeros(m,n);
for i=-RANGE:1:RANGE
    fix_mat_55(55/PRECISION_RES_ANGLE+i,:)=linspace(FIX_LO,1,n);
end
fix_mat_55=imfilter(fix_mat_55,Filter1,'replicate');
fix_mat_55=imfilter(fix_mat_55,Filter2,'replicate');

fix_mat_65=zeros(m,n);
for i=-RANGE:1:RANGE
    fix_mat_65(65/PRECISION_RES_ANGLE+i,:)=linspace(FIX_LO,1,n);
end
fix_mat_65=imfilter(fix_mat_65,Filter1,'replicate');
fix_mat_65=imfilter(fix_mat_65,Filter2,'replicate');

fix_mat_75=zeros(m,n);
for i=-RANGE:1:RANGE
    fix_mat_75(75/PRECISION_RES_ANGLE+i,:)=linspace(FIX_LO,1,n);
end
fix_mat_75=imfilter(fix_mat_75,Filter1,'replicate');
fix_mat_75=imfilter(fix_mat_75,Filter2,'replicate');

fix_mat_85=zeros(m,n);
for i=-RANGE:1:RANGE
    fix_mat_85(85/PRECISION_RES_ANGLE+i,:)=linspace(FIX_LO,1,n);
end
fix_mat_85=imfilter(fix_mat_85,Filter1,'replicate');
fix_mat_85=imfilter(fix_mat_85,Filter2,'replicate');

fix_mat_tot=fix_mat_15+fix_mat_25+fix_mat_35+fix_mat_45+fix_mat_55+fix_mat_65+fix_mat_75+fix_mat_85;

Zo_final=Zo_fitted+FIX_RATIO_15*fix_mat_15;
Zo_final=Zo_final+FIX_RATIO_25*fix_mat_25;
Zo_final=Zo_final+FIX_RATIO_35*fix_mat_35;
Zo_final=Zo_final+FIX_RATIO_45*fix_mat_45;
Zo_final=Zo_final+FIX_RATIO_55*fix_mat_55;
Zo_final=Zo_final+FIX_RATIO_65*fix_mat_65;
Zo_final=Zo_final+FIX_RATIO_75*fix_mat_75;
Zo_final=Zo_final+FIX_RATIO_85*fix_mat_85;

%-------------------------------------------------------------------------
%Display
[m1,n1]=size(Zi);
Size1=ones(m1,1)*12;
Color1=[ones(m1,1)*1.0,ones(m1,1)*0.0,ones(m1,1)*0.0];
[m2,n2]=size(Zi2);
Size2=ones(m2,1)*12;
Color2=[ones(m2,1)*0.0,ones(m2,1)*1.0,ones(m2,1)*0.3];

close all;
figure;
hold on;
scatter3(Xi,Yi,Zi*FIX_RATIO_GLOBAL,Size1,Color1);
scatter3(Xi2,Yi2,Zi2*FIX_RATIO_GLOBAL,Size2,Color2);
mesh(Xo,Yo,Zo_final);
hold off;
axis([RANGE_SPEED_LO,RANGE_SPEED_HI,RANGE_ANGLE_LO,RANGE_ANGLE_HI]);
xlabel('Speed'),ylabel('Angle'),zlabel('FixAngle');

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
strName='CompensationResult';
strFileName=[strName,'-Car',strNum,'.xlsx'];
[stat,msg]=xlswrite(strFileName, Zf, 'Compensation_Result');
