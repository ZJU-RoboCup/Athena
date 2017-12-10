function [cycle,u,z,Out_bel,bayesfilename] = ReadOutputFile(outputfilename)
    fid = fopen(outputfilename, 'r');
    if fid >0
        C_test = textscan(fid, '%s',1,'commentStyle','#');
        C_data = textscan(fid, '%d %d %d %f %f %f','delimiter', '\t',...
                                            'commentStyle','#');
    end
    fclose(fid);
    bayesfilename = cell2mat(C_test{1});
    cycle = cell2mat(C_data(:,1));
    u = cell2mat(C_data(:,2))+1;
    z = cell2mat(C_data(:,3))+1;
    Out_bel = cell2mat(C_data(:,4:6));
end