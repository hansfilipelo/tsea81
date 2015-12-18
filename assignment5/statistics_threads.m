statFolder = 'one_thread_ass3';
statFolder2 = 'two_threads_ass3';
statFolder3 = 'four_threads_ass3';
statFolder4 = 'ass3';
cores = [1,2,4,8];
stdDev = [];
myMean = [];


A = load([statFolder '/stat_' num2str(70) '.txt']);
B = load([statFolder2 '/stat_' num2str(70) '.txt']);
C = load([statFolder3 '/stat_' num2str(70) '.txt']);
D = load([statFolder4 '/stat_' num2str(70) '.txt']);

tmpDev = sqrt(var(A));
stdDev = [stdDev tmpDev];
% Queue version
tmpDev2 = sqrt(var(B));
stdDev = [stdDev tmpDev2];
% Optimized version
tmpDev3 = sqrt(var(C));
stdDev = [stdDev tmpDev3];
tmpDev4 = sqrt(var(D));
stdDev = [stdDev tmpDev4];

tmpMean = mean(A);
myMean = [myMean tmpMean];
% Queue version
tmpMean2 = mean(B);
myMean = [myMean tmpMean2];
% Optimized version
tmpMean3 = mean(C);
myMean = [myMean tmpMean3];
tmpMean4 = mean(D);
myMean = [myMean tmpMean4];

figure(1)
plot(cores,stdDev,'b')
legend('StdDev nr of cores');
title('Standard deviations');
xlabel('Cores (nr)');
ylabel('StdDev');

figure(2)
plot(cores,myMean,'b')
legend('Mean nr of cores');
title('Average lift travel times');
xlabel('Cores (nr)');
ylabel('Avg travel times');
