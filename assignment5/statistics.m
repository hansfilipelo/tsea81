statFolder = 'ass3';
statFolder2 = 'mod_ass3';
passengers = [10, 20, 30, 40, 50, 60, 70];
stdDev = [];
myMean = [];
stdDev2 = [];
myMean2 = [];

for i = 1:length(passengers)
  
  A = load([statFolder '/stat_' num2str(passengers(i)) '.txt']);
  B = load([statFolder2 '/stat_' num2str(passengers(i)) '.txt']);
  
  %disp(['Standard deviation ' num2str(passengers(i))]);
  tmpDev = sqrt(var(A));
  stdDev = [stdDev tmpDev];
  % Optimized version
  tmpDev2 = sqrt(var(B));
  stdDev2 = [stdDev2 tmpDev2];
  
  %disp(['Mean ' num2str(passengers(i))]);
  tmpMean = mean(A);
  myMean = [myMean tmpMean];
  % Optimized version
  tmpMean2 = mean(B);
  myMean2 = [myMean2 tmpMean2];
 
  %disp('');
end

figure(1)
plot(passengers,stdDev,'b',passengers,stdDev2,'r')
legend('Un-optimized version','Optimized version');
title('Standard deviations Ass3');
xlabel('Number of passengers');
ylabel('StdDev [usec]');

figure(2)
plot(passengers,myMean,'b',passengers,myMean2,'r')
legend('Un-optimized version','Optimized version');
title('Average lift travel times Ass3');
xlabel('Number of passengers');
ylabel('Avg travel times [usec]');


