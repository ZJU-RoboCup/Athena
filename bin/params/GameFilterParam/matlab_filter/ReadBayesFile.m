function [Pxux,Pzx,Init_bel]= ReadBayesFile(filename)
    fid = fopen(filename, 'r');
    Pxux = zeros(3,3,3);
    Pzx = zeros(3,3);
    Init_bel = zeros(1,3);
    if fid >0
            C = textscan(fid, '%s','commentStyle','#');
            C = C{1};
            iterC = 1;
            while iterC <= length(C)
                temp_state = C{iterC};
                iterC = iterC+1;
                if 1 == strcmpi(temp_state,'pxux')
                    numstate = 0;
                    while numstate < 9
                        temp_gamestate1 = C{iterC};
                        iterC = iterC + 1;
                        temp_gamestate2 = C{iterC};
                        iterC = iterC + 1;
                        last_x = string2state(temp_gamestate1);
                        u = string2state(temp_gamestate2);
                        if last_x == -1 || u == -1
                            error('Bayes:ReadEorror',...
                                  'Error In BayesReader\n %s %s\n',temp_gamestate1,temp_gamestate2);
                        end
                        Pxux(last_x,u,1) = str2double(C{iterC});
                        iterC = iterC + 1;
                        Pxux(last_x,u,2) = str2double(C{iterC});
                        iterC = iterC + 1;
                        Pxux(last_x,u,3) = str2double(C{iterC});
                        iterC = iterC + 1;
                        if abs(sum(Pxux(last_x,u,:)) - 1) > 1e-6
                            error('Bayes:ReadEorror',...
                                  'Error In BayesParam Pxux.\n%s %s\n',temp_gamestate1,temp_gamestate2);
                        end
                        numstate = numstate + 1;
                    end
                elseif 1 == strcmpi(temp_state,'pzx')
                    numstate = 0;
                    while numstate < 3
                         temp_gamestate = C{iterC};
                         iterC = iterC + 1;
                         z = string2state(temp_gamestate);
                         if z == -1
                             error('Bayes:ReadEorror',...
                                   'Error In BayesReader\n %s \n',temp_gamestate);
                         end
                          Pzx(z,1) = str2double(C{iterC});
                          iterC = iterC + 1;
                          Pzx(z,2) = str2double(C(iterC));
                          iterC = iterC + 1;
                          Pzx(z,3) = str2double(C(iterC));
                          iterC = iterC + 1;
                          if abs(sum(Pzx(z,:)) - 1) > 1e-6
                               error('Bayes:ReadEorror',...
                                     'Error In BayesParam Pzx.\n%s \n',temp_gamestate);
                          end
                          numstate = numstate + 1;
                    end
                elseif 1 == strcmpi(temp_state,'init')
                    numstate = 0;
                    while numstate <3
                         temp_gamestate = C{iterC};
                         iterC = iterC + 1;
                         x = string2state(temp_gamestate);
                         if x == -1
                             error('Bayes:ReadEorror',...
                                   'Error In BayesParam Init.\n%s \n',temp_gamestate);
                         end
                         Init_bel(x) = str2double(C(iterC));
                         iterC = iterC + 1;
                         numstate = numstate + 1;
                    end
                else
                     error('Bayes:ReadEorror',...
                           'Error In BayesParam.\n%s \n',temp_state);
                end
            end
    end
    fclose(fid);
end

function val = string2state(state)
    state = lower(state);
    if strcmpi(state,'attack')
        val = 1;
    elseif strcmpi(state,'deadlock')
        val = 2;
    elseif strcmpi(state,'defend')
        val = 3;
    else
        val = -1;
    end
end