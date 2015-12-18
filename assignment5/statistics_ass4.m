statFolder = 'ass4';
statFolder2 = 'queue_ass4';
statFolder3 = 'mod_ass4';
passengers = [10, 20, 30, 40, 50, 60, 70];
stdDev = [];
myMean = [];
stdDev2 = [];
myMean2 = [];
stdDev3 = [];
myMean3 = [];

for i = 1:length(passengers)
  
  A = load([statFolder '/stat_' num2str(passengers(i)) '.txt']);
  B = load([statFolder2 '/stat_' num2str(passengers(i)) '.txt']);
  C = load([statFolder3 '/stat_' num2str(passengers(i)) '.txt']);
  
  %disp(['Standard deviation ' num2str(passengers(i))]);
  tmpDev = sqrt(var(A));
  stdDev = [stdDev tmpDev];
  % Queue version
  tmpDev2 = sqrt(var(B));
  stdDev2 = [stdDev2 tmpDev2];
  % Optimized version
  tmpDev3 = sqrt(var(C));
  stdDev3 = [stdDev3 tmpDev3];
  
  %disp(['Mean ' num2str(passengers(i))]);
  tmpMean = mean(A);
  myMean = [myMean tmpMean];
  % Queue version
  tmpMean2 = mean(B);
  myMean2 = [myMean2 tmpMean2];
  % Optimized version
  tmpMean3 = mean(C);
  myMean3 = [myMean3 tmpMean3];
  %disp('');
end

figure(1)
plot(passengers,stdDev,'b',passengers,stdDev2,'r',passengers,stdDev3,'k')
legend('Un-optimized version','Queue version','Optimized version');
title('Standard deviations Ass4');
xlabel('Number of passengers');
ylabel('StdDev [usec]');

figure(2)
plot(passengers,myMean,'b',passengers,myMean2,'r',passengers,myMean3,'k')
legend('Un-optimized version','Queue version','Optimized version');
title('Average lift travel times Ass4');
xlabel('Number of passengers');
ylabel('Avg travel times [usec]');
