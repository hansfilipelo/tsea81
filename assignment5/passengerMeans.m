statFolderAss3 = 'mod_ass3';
statFolderAss4 = 'mod_ass4';

nrOfPassengers = 70;
nrOfIterations = 10000;

passengerNo = 0:nrOfPassengers - 1;
passengerMeansAss3 = zeros(nrOfPassengers,1);
passengerMeansAss4 = zeros(nrOfPassengers,1);
passengerMaxTimesAss3 = zeros(nrOfPassengers,1);
passengerMaxTimesAss4 = zeros(nrOfPassengers,1);
passengerMinTimesAss3 = zeros(nrOfPassengers,1);
passengerMinTimesAss4 = zeros(nrOfPassengers,1);

A = load([statFolderAss3 '/stat_70.txt']);
B = load([statFolderAss4 '/stat_70.txt']);

for i = 1:nrOfPassengers
   
    j = i-1;
    passengerMeansAss3(i) = mean(A(j*nrOfIterations + 1: i*nrOfIterations));
    passengerMeansAss4(i) = mean(B(j*nrOfIterations + 1: i*nrOfIterations));
    passengerMaxTimesAss3(i) = max(A(j*nrOfIterations + 1: i*nrOfIterations));
    passengerMaxTimesAss4(i) = max(B(j*nrOfIterations + 1: i*nrOfIterations));
    passengerMinTimesAss3(i) = min(A(j*nrOfIterations + 1: i*nrOfIterations));
    passengerMinTimesAss4(i) = min(B(j*nrOfIterations + 1: i*nrOfIterations));
end

figure(1)
plot(passengerNo, passengerMinTimesAss3,'b',passengerNo, passengerMeansAss3,'r', passengerNo, passengerMaxTimesAss3,'k');
legend('Min', 'Average','Max');
title('Min, Average and Max travel times for each passenger for modified Ass3 with 70 passengers');
xlabel('Passengerid');
ylabel('Time [usec]');
%axis([0 70 0 7500]);

figure(2)
plot(passengerNo, passengerMinTimesAss4,'b',passengerNo, passengerMeansAss4,'r', passengerNo, passengerMaxTimesAss4,'k');
legend('Min', 'Average','Max');
title('Min, Average and Max travel times for each passenger for modified Ass4 with 70 passengers');
xlabel('Passengerid');
ylabel('Time [usec]');
%axis([0 70 0 150]);
