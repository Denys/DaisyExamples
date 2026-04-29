# Hack Audio Textbook Code LLM Reference

Source directory: `Hack_Audio_textbookcode-master/`

Scope: all MATLAB `.m` source files are embedded verbatim, grouped by chapter. Non-code assets such as `.wav` files are listed in each chapter manifest and are not inlined.

Source notice: this folder identifies the material as code from the Hack Audio textbook, published by Taylor & Francis for educational use. See `Hack_Audio_textbookcode-master/README.md` for the original notice and links.

LLM navigation markers:

- Search for `BEGIN_CHAPTER: Ch_XX` to jump to a chapter.
- Search for `BEGIN_FILE: Ch_XX/name.m` to jump to a specific source file.
- Each source block is fenced as MATLAB and bounded by `BEGIN_FILE` / `END_FILE` comments.

## Corpus Summary

| Item | Count |
|---|---:|
| Chapters | 16 |
| MATLAB source files | 153 |
| Non-code assets | 16 |
| Embedded source bytes | 233550 |

## Chapter Index

| Chapter | Code files | Assets |
|---|---:|---:|
| `Ch_03` | 0 | 3 |
| `Ch_04` | 1 | 0 |
| `Ch_05` | 3 | 0 |
| `Ch_06` | 6 | 0 |
| `Ch_07` | 10 | 0 |
| `Ch_08` | 11 | 0 |
| `Ch_09` | 6 | 0 |
| `Ch_10` | 14 | 0 |
| `Ch_11` | 8 | 1 |
| `Ch_12` | 5 | 0 |
| `Ch_13` | 11 | 6 |
| `Ch_14` | 9 | 0 |
| `Ch_15` | 29 | 0 |
| `Ch_16` | 15 | 0 |
| `Ch_17` | 10 | 3 |
| `Ch_18` | 15 | 3 |

<!-- BEGIN_CHAPTER: Ch_03 -->
## Ch_03

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| asset | `Ch_03/sw20Hz.wav` | - | 88246 |
| asset | `Ch_03/sw440Hz.wav` | - | 441046 |
| asset | `Ch_03/Vocal.wav` | - | 430932 |

### Source Files

<!-- END_CHAPTER: Ch_03 -->

<!-- BEGIN_CHAPTER: Ch_04 -->
## Ch_04

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_04/simpleAddition.m` | 19 | 638 |

### Source Files

<!-- BEGIN_FILE: Ch_04/simpleAddition.m -->
#### `Ch_04/simpleAddition.m`

````matlab
a = 3
b = 2
c = a+b

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_04/simpleAddition.m -->

<!-- END_CHAPTER: Ch_04 -->

<!-- BEGIN_CHAPTER: Ch_05 -->
## Ch_05

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_05/simpleLoop.m` | 38 | 1470 |
| code | `Ch_05/userSqrt.m` | 27 | 840 |
| code | `Ch_05/userSqrtExample.m` | 26 | 974 |

### Source Files

<!-- BEGIN_FILE: Ch_05/simpleLoop.m -->
#### `Ch_05/simpleLoop.m`

````matlab
% This script compares the process of using a "for" loop
% with the process of using a "while" loop. The same
% result can be accomplished with each type loop by using
% the appropriate syntax.

% FOR Loop Example
N = 4;               % Number of elements for signal
sig1 = zeros(N,1);   % Initialize an output signal
for ii = 1:N
    sig1(ii) = sin(ii*pi/2);  % Fill the output array
                              % with values from the
end                           % sine function
sig1                 % Display result on Command Window

% WHILE Loop Example
sig2 = zeros(N,1);   % Initialize new output signal
jj = 1;              % Declare a counting variable
while jj <= N 
    sig2(jj) = sin(jj*pi/2);  % Fill the output array
    jj = jj + 1;              % Iterate counting variable
end
sig2                 % Display result on Command Window

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_05/simpleLoop.m -->

<!-- BEGIN_FILE: Ch_05/userSqrt.m -->
#### `Ch_05/userSqrt.m`

````matlab
% USERSQRT Calculate the square root.
%    y = userSqrt(x) calculates the square root of 'x'.
%
%	 x : scalar input variable
%    y : scalar output variable
%
%    See also SQRT

function [y] = userSqrt(x)

y = (x)^(1/2);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_05/userSqrt.m -->

<!-- BEGIN_FILE: Ch_05/userSqrtExample.m -->
#### `Ch_05/userSqrtExample.m`

````matlab
in1 = 36;              % Declare a few variables
in2 = 144;             % with different values
in3 = 16;

% Calculate the square-root of the variables
out1 = userSqrt(in1)   % Call the userSqrt function

out2 = userSqrt(in2)   % Print the result to demonstrate
                       % the function works for various
out3 = userSqrt(in3)   % input values

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_05/userSqrtExample.m -->

<!-- END_CHAPTER: Ch_05 -->

<!-- BEGIN_CHAPTER: Ch_06 -->
## Ch_06

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_06/characteristicCurve.m` | 58 | 1862 |
| code | `Ch_06/dbAmpChange.m` | 28 | 971 |
| code | `Ch_06/dbAmpExample.m` | 38 | 1123 |
| code | `Ch_06/dcOffset.m` | 59 | 1703 |
| code | `Ch_06/elementLoop.m` | 41 | 1185 |
| code | `Ch_06/scaleAmp.m` | 58 | 1581 |

### Source Files

<!-- BEGIN_FILE: Ch_06/characteristicCurve.m -->
#### `Ch_06/characteristicCurve.m`

````matlab
% CHARACTERISTICCURVE
% This script provides two examples for analyzing the 
% characteristic curve of an audio effect which uses 
% element-wise processing
%
% This first example creates an input array with values 
% from -1 to 1. The second example uses a sine wave signal

clear;clc;close all;

% Example 1: Array [-1 to 1] to span entire full-scale range
in = [-1:.1:1].';

% Example 2: Sine Wave Test Signal. This example shows the
% characteristic curve can be created using any signal
% which spans the full-scale range, even if it is periodic.
% Uncomment this code to switch examples.
%[in,Fs] = audioread('sw20Hz.wav');

% Assign in to out1, for comparison purposes
out1 = in;   % No amplitude change

N = length(in);
out2 = zeros(N,1);
% Loop through arrays to perform element-wise multiplication
for n = 1:N
    out2(n,1) = 2*in(n,1);
end

% Element-wise multiplication can also be 
% accomplished by multiplying an array directly by a scalar
out3 = 3*in;

% Plot the Characteristic Curve (In vs. Out)
plot(in,out1,in,out2,in,out3);
xlabel('Input Amplitude');
ylabel('Output Amplitude');
legend('out1 = in','out2 = 2*in','out3 = 3*in');
% Draw axes through origin
ax = gca;
ax.XAxisLocation = 'origin';
ax.YAxisLocation = 'origin';

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_06/characteristicCurve.m -->

<!-- BEGIN_FILE: Ch_06/dbAmpChange.m -->
#### `Ch_06/dbAmpChange.m`

````matlab
%DBAMPCHANGE This function changes the amplitude of a signal
%   This function changes the amplitude of an input signal based on a
%   desired change relative to a decibel scale

function [ out ] = dbAmpChange( in,dBChange )

scale = 10^(dBChange/20); % Convert from decible to linear

out = scale*in;           % Apply linear amplitude to signal


end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_06/dbAmpChange.m -->

<!-- BEGIN_FILE: Ch_06/dbAmpExample.m -->
#### `Ch_06/dbAmpExample.m`

````matlab
% DBAMPEXAMPLE
% This script provides an example for changing the amplitude 
% of a signal on a decibel (dB) scale
%
% See also DBAMPCHANGE

% Example - Sine wave test signal
[sw1,Fs] = audioread('sw20Hz.wav');
sw2 = dbAmpChange(sw1,6);
sw3 = dbAmpChange(sw1,-6);

% Plot the result
Ts = 1/Fs;
N = length(sw1);
t = [0:N-1]*Ts; t=t(:);
plot(t,sw1,'.',t,sw2,t,sw3,'--');
xlabel('Time (sec.)');
ylabel('Amplitude');
title('Changing the Amplitude of a Signal on a dB Scale');
legend('SW1','+6 dB','-6 dB');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_06/dbAmpExample.m -->

<!-- BEGIN_FILE: Ch_06/dcOffset.m -->
#### `Ch_06/dcOffset.m`

````matlab
% DCOFFSET
% This script demonstrates a method to perform
% element-wise scalar addition, the equivalent
% of a DC offset

clear;clc;close all;

% Example - Sine Wave Signal
filename = 'sw20Hz.wav';
[in,Fs] = audioread(filename); % Import Sound File
Ts = 1/Fs;
% Assign in to out1, for comparison purposes
out1 = in;

N = length(in);
out2 = zeros(N,1); % Initialize output array
% Loop through arrays to perform element-wise scalar addition
for n = 1:N
    out2(n,1) = in(n,1)+1;
end

% Element-wise addition can also be 
% accomplished by adding a scalar directly to an array
out3 = in-0.5;

figure; % Create a new figure window
% Plot the output amplitude vs. time
t = [0:N-1]*Ts; t = t(:);
plot(t,out1,t,out2,t,out3);
xlabel('Time (sec.)');
ylabel('Amplitude');
legend('out1 = in','out2 = in+1','out3 = in-0.5');

figure;
% Plot the Input vs. Output
plot(in,out1,in,out2,in,out3);
xlabel('Input Amplitude');
ylabel('Output Amplitude');
legend('out1 = in','out2 = in+1','out3 = in-0.5');
% Draw axes through origin
ax = gca;
ax.XAxisLocation = 'origin';
ax.YAxisLocation = 'origin';

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_06/dcOffset.m -->

<!-- BEGIN_FILE: Ch_06/elementLoop.m -->
#### `Ch_06/elementLoop.m`

````matlab
% ELEMENTLOOP
% This script demonstrates a method to reference
% and assign the elements of an input array to an 
% output array
%
% A loop is used to step through each element of
% the arrays

clear; clc;
% Example - Sine wave test signal
filename = 'sw20Hz.wav';
[x,Fs] = audioread(filename); % input signal

N = length(x);
y = zeros(N,1);    
% Loop through arrays to perform element-wise referencing
% n - variable for sample number
for n = 1:N
    % Note: a new element from 'x' is assigned to 'y'
    % each time through the loop
    y(n,1) = x(n,1)
    
end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_06/elementLoop.m -->

<!-- BEGIN_FILE: Ch_06/scaleAmp.m -->
#### `Ch_06/scaleAmp.m`

````matlab
% SCALEAMP
% This script demonstrates two methods for 
% scaling the amplitude of a signal. The first
% method uses a loop to perform element-wise
% indexing of the input signal. The second method
% uses MATLAB's scalar operation with the array.

clear; clc;

% Import the input signal
filename = 'sw20Hz.wav';
[x,Fs] = audioread(filename); % input signal

Ts = 1/Fs;
% Time vector for plotting
t = [0:length(x)-1]*Ts; t = t(:);

% Example 1 - Loop
g1 = 0.5; % Gain Scalar

N = length(x);
y1 = zeros(N,1);
% n - variable for sample number
for n = 1:N
    % Multiply each element of "x" by "g1"
    y1(n,1) = g1 * x(n,1);
    
end

figure(1); % Plot Example 1
plot(t,x,'--',t,y1); legend('x','0.5*x');

% Example 2 - Array Operation
g2 = 0.25;

% In this approach, it is not necessary to use 
% a loop to index the individual elements of "x".
% By default, this operation performs element-wise processing
y2 = g2 * x;

figure(2);  % Plot Example 2
plot(t,x,t,y2);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_06/scaleAmp.m -->

<!-- END_CHAPTER: Ch_06 -->

<!-- BEGIN_CHAPTER: Ch_07 -->
## Ch_07

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_07/dutyCycle.m` | 33 | 945 |
| code | `Ch_07/impulseTrain.m` | 53 | 1668 |
| code | `Ch_07/plottf.m` | 50 | 1561 |
| code | `Ch_07/sawtoothSynthesis.m` | 40 | 1074 |
| code | `Ch_07/sineAngle.m` | 54 | 1466 |
| code | `Ch_07/sineSpectrum.m` | 39 | 1155 |
| code | `Ch_07/sineSynthesis.m` | 59 | 1747 |
| code | `Ch_07/squareSynthesis.m` | 41 | 1155 |
| code | `Ch_07/triangleSynthesis.m` | 49 | 1514 |
| code | `Ch_07/whiteNoiseSpectrum.m` | 30 | 947 |

### Source Files

<!-- BEGIN_FILE: Ch_07/dutyCycle.m -->
#### `Ch_07/dutyCycle.m`

````matlab
% DUTYCYCLE
% This script synthesizes a square wave
% with an asymmetrical duty cycle
%
% See also SQUARESYNTHESIS
clear;clc;
% Example - Duty Cycle = 25
Fs = 1000;
Ts = (1/Fs);
t = 0:Ts:1;
t = t(:);
f = 3;
duty = 25;
sq = square(2*pi*f.*t, duty); 
plot(t,sq); axis([0 1 -1.1 1.1]);
xlabel('Time (sec.)');ylabel('Amplitude');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_07/dutyCycle.m -->

<!-- BEGIN_FILE: Ch_07/impulseTrain.m -->
#### `Ch_07/impulseTrain.m`

````matlab
% IMPULSETRAIN
% This script demonstrates a method to create 
% an Impulse Train signal. Initially, all values 
% of the signal are set to zero. Then, individual
% samples are changed to a value of 1 based on the
% length of a cycle's period.
clear;clc;

% Example - Impulse Train Signal
% 5 Hz signal for visualization
f = 5;
Fs = 20; Ts = (1/Fs); 
t = 0:Ts:1; t = t(:); % Time vector

impTrain = zeros(size(t)); % Initialize to all zeros
period = round(Fs/f); % # of samples/cycle
% Change the single sample at the start of a cycle to 1
impTrain(1:period:end) = 1;
stem(t,impTrain); axis([0 1 -0.1 1.1]); % Show stem plot

% Example - 440 Hz signal for audition
f = 440; 
Fs = 48000; Ts = (1/Fs);
t = 0:Ts:3; t = t(:);
it = zeros(size(t));
period = round(Fs/f); % # of samples/cycle
it(1:period:end) = 1;
sound(it,Fs);  % Listen to signal

% 50 Hz signal for spectrum plot
f = 50; 
Fs = 48000; Ts = (1/Fs);
t = 0:Ts:1; t = t(:);
it = zeros(size(t));
period = round(Fs/f); % # of samples/cycle
it(1:period:end) = 1;
plottf(it,Fs);   % Plot spectrum

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_07/impulseTrain.m -->

<!-- BEGIN_FILE: Ch_07/plottf.m -->
#### `Ch_07/plottf.m`

````matlab
%PLOTTF Plot sampled signal in time and frequency domains
%   PLOTTF(x,Fs) plots the time-domain samples in vector x, assuming that 
%   Fs is an audio sampling rate (44.1k, 48k, etc.) in samples/second, 
%   and also plots the the Fourier transform on the decibel scale
%   between the frequencies of 20 Hz and 20 kHz, logarithmically spaced.
%
% See also PLOT

function plottf(x,Fs)
Ts = 1/Fs;
N = length(x);
t = [0:N-1]*Ts; t=t(:);

subplot(2,1,1);
plot(t,x); xlabel('Time (sec.)'); ylabel('Amplitude');

% Fourier Transform
len = N;
if len < 4096
    len = 4096;
end
X=(2/N)*fft(x,len);        % do DFT/FFT

f= [0:len-1]*(Fs/len);

% Ensure there will be no values of -Inf dB
% by making the minimum value = -120 dB
X(abs(X)<0.000001) = 0.000001;

subplot(2,1,2);
semilogx(f,20*log10(abs(X))); axis([20 20000 -60 4]);
ax = gca; ax.XTick =[20 50 100 200 500 1000 2000 5000 10000 20000];
xlabel('Frequency (Hz)'); ylabel('Amplitude (dB)');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_07/plottf.m -->

<!-- BEGIN_FILE: Ch_07/sawtoothSynthesis.m -->
#### `Ch_07/sawtoothSynthesis.m`

````matlab
% SAWTOOTHSYNTHESIS
% This script demonstrates the 'sawtooth' function
%
% See also SINESYNTHESIS, SQUARESYNTHESIS, TRIANGLESYNTHESIS

clear;clc;
% Example - Sawtooth Wave Signal
% 4 Hz signal for visualization
f = 4;
phi = 0;
Fs = 40;
Ts = (1/Fs);
t = [0:Ts:1].';
sawtoothWave = sawtooth(2*pi*f.*t + phi);
plot(t,sawtoothWave);

% 880 Hz signal for audition
Fs = 44100;
Ts = (1/Fs);
t = 0:Ts:3;
t = t(:);
f = 880;
st = sawtooth(2*pi*f.*t); 
sound(st,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_07/sawtoothSynthesis.m -->

<!-- BEGIN_FILE: Ch_07/sineAngle.m -->
#### `Ch_07/sineAngle.m`

````matlab
% SINEANGLE
% This script demonstrates a method to synthesize
% sine waves by using an angle of rotation
%
% See also SINESYNTHESIS

clear;clc;
% Declare initial parameters
f = 2; % Frequency in Hz
phi = 0; % phase offset
Fs = 1000; % sampling rate
Ts = 1/Fs; % sample period
t = [0:Ts:1].'; % sample times

% Calculate Angle of Rotation
angleChange = f*Ts*2*pi;
currentAngle = phi;

N = length(t);
out = zeros(N,1);
% Update the value of the currentAngle each iteration
% through the loop
for n = 1:N
    
    out(n,1) = sin(currentAngle);
    % Update phase angle for next loop
    currentAngle = currentAngle + angleChange;
    
    if currentAngle > 2*pi % Ensure angle is not > 2*pi
        currentAngle = currentAngle - 2*pi;
    end
end


% Plot the synthesized signal
plot(t,out);
xlabel('Time (sec.)'); ylabel('Amplitude');
legend('out');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_07/sineAngle.m -->

<!-- BEGIN_FILE: Ch_07/sineSpectrum.m -->
#### `Ch_07/sineSpectrum.m`

````matlab
% SINESPECTRUM
% This script shows the waveform and frequency spectrum
% plots for a sine wave
%
% See also PLOTTF, SINESYNTHESIS

clear; clc; close all;
% Example - Sine Wave Signal
% Declare initial parameters
f = 50; % Frequency in Hz
Fs = 48000; Ts = 1/Fs;
lenSec = 1; % 1 second long signal
N = Fs*lenSec;  % Convert to time samples
x = zeros(N,1);
% Loop to perform element-wise referencing
for n = 1:N
    t = (n-1) * Ts;   % 't' is a scalar
    x(n,1) = sin(2*pi*f*t); 
end

% Plot Waveform and Frequency Spectrum
plottf(x,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_07/sineSpectrum.m -->

<!-- BEGIN_FILE: Ch_07/sineSynthesis.m -->
#### `Ch_07/sineSynthesis.m`

````matlab
% SINESYNTHESIS
% This script demonstrates two methods to synthesize
% sine waves
%
% Method 1: A loop is used to step through each 
% element of the arrays
%
% Method 2: Array processing is used to perform
% element-wise referencing by the 'sin' function 
% internally
%
% See also SINEANGLE, SINESPECTRUM

clear; clc;
% Example - Sine Wave Signal
% Declare initial parameters
f = 2; % Frequency in Hz
phi = 0; % phase offset
Fs = 100; % sampling rate
Ts = 1/Fs; % sampling period
lenSec = 1; % 1 second long signal
N = Fs*lenSec;  % Convert to time samples
out1 = zeros(N,1);
% Method 1: Loop to perform element-wise referencing
for n = 1:N
    % Note: a new time 't' is used
    % each time through the loop
    t = (n-1) * Ts;   % 't' is a scalar
     
    out1(n,1) = sin(2*pi*f*t+phi); 
    
end

% Method 2: Create signal using array processing
% Phase shifted signal of identical frequency
t = [0:(N-1)] * Ts;  % sample times
t = t(:); % make a column vector
out2 = sin(2*pi*f*t+pi/2);  % 90 degree phase shift

% Plot the 2 signals
plot(t,out1,t,out2);
xlabel('Time (sec.)'); ylabel('Amplitude');
legend('out1','out2');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_07/sineSynthesis.m -->

<!-- BEGIN_FILE: Ch_07/squareSynthesis.m -->
#### `Ch_07/squareSynthesis.m`

````matlab
% SQUARESYNTHESIS
% This script demonstrates the 'square' function
%
% See also SINESYNTHESIS, SAWTOOTHSYNTHESIS, TRIANGLESYNTHESIS

clear;clc;
% Example - Square Wave Signal
% 2 Hz square wave for visualization
f = 2; 
phi = 0;
Fs = 1000;
Ts = (1/Fs);
t = [0:Ts:1].';
squareWave = square(2*pi*f.*t + phi);
plot(t,squareWave); axis([0 1 -1.1 1.1]);
xlabel('Time (sec.)');ylabel('Amplitude');
legend('2 Hz');

% 880 Hz square wave for audition
Fs = 44100;
Ts = (1/Fs);
t = 0:Ts:3; t = t(:);
f = 880;
sq = square(2*pi*f.*t);
sound(sq,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_07/squareSynthesis.m -->

<!-- BEGIN_FILE: Ch_07/triangleSynthesis.m -->
#### `Ch_07/triangleSynthesis.m`

````matlab
% TRIANGLESYNTHESIS
% This script demonstrates a method
% to transform the 'sawtooth' function to
% a triangle wave
%
% See also SINESYNTHESIS, SAWTOOTHSYNTHESIS, SQUARESYNTHESIS

clear;clc;
% Example - Triangle Wave Signal
% 4 Hz signal for visualization
f = 4; phi = 0; Fs = 40;
Ts = 1/Fs;
t = [0:Ts:1].'; % Time vector in seconds

% Triangle wave => peak occurs at half (0.5) of cycle length
triangleWave = sawtooth(2*pi*f.*t + phi,0.5);
plot(t,triangleWave);                 

% Sawtooth with peak at beginning of cycle, then decreasing amp
sawtoothWave = sawtooth(2*pi*f.*t + phi,0);
plot(t,sawtoothWave);

% Sawtooth with increasing amp during cycle, peak at end 
sawtoothWave = sawtooth(2*pi*f.*t + phi,1);
plot(t,sawtoothWave);

% 880 Hz signal for audition
Fs = 44100; Ts = (1/Fs);
t = 0:Ts:3; t = t(:);
f = 880; % Frequency in Hz

tr = sawtooth(2*pi*f.*t,0.5); % Triangle
sound(tr,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_07/triangleSynthesis.m -->

<!-- BEGIN_FILE: Ch_07/whiteNoiseSpectrum.m -->
#### `Ch_07/whiteNoiseSpectrum.m`

````matlab
% WHITENOISESPECTRUM
% This script shows the waveform and frequency spectrum
% plots for white noise
%
% See also PLOTTF

clear; clc; close all;
% Example - White Noise
% Declare initial parameters
Fs = 48000;
lenSec = 1; % 1 second long signal
N = Fs*lenSec;  % Convert to time samples
noise = 0.2*randn(N,1);
plottf(noise,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_07/whiteNoiseSpectrum.m -->

<!-- END_CHAPTER: Ch_07 -->

<!-- BEGIN_CHAPTER: Ch_08 -->
## Ch_08

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_08/additionExample.m` | 67 | 1866 |
| code | `Ch_08/ampModulation.m` | 58 | 1736 |
| code | `Ch_08/equalFades.m` | 38 | 1224 |
| code | `Ch_08/exponentialFade.m` | 68 | 1721 |
| code | `Ch_08/linearFade.m` | 54 | 1486 |
| code | `Ch_08/morphLFO.m` | 53 | 1615 |
| code | `Ch_08/plottf.m` | 49 | 1545 |
| code | `Ch_08/ringModulation.m` | 43 | 1164 |
| code | `Ch_08/sCurveFade.m` | 44 | 1381 |
| code | `Ch_08/sineCurveFade.m` | 43 | 1370 |
| code | `Ch_08/subtractionExample.m` | 54 | 1724 |

### Source Files

<!-- BEGIN_FILE: Ch_08/additionExample.m -->
#### `Ch_08/additionExample.m`

````matlab
% ADDITIONEXAMPLE
% This script provides two examples for combining 
% signals together using addition
%
% This first example is for signals of the 
% same frequency and phase
%
% The second example is for signals of different frequencies
%
% See also SUBTRACTIONEXAMPLE

% Example 1 - Same Frequencies
% Declare initial parameters
f = 1; 
a = 1;
phi = 0;
Fs = 100;
t = [0:1/Fs:1].';
sw1 = a * sin(2*pi*f.*t+phi);
sw2 = a * sin(2*pi*f.*t+phi);
N = length(sw1);
sw3 = zeros(N,1);
% Loop through arrays to perform element-wise addition
for n = 1:N
    sw3(n,1) = sw1(n,1)+sw2(n,1);
end
% Plot the result
plot(t,sw1,'--',t,sw2,'o',t,sw3);
xlabel('Time (sec.)');
ylabel('Amplitude');
title('Addition of 2 Sine Waves - Same Frequencies');
legend('SW1','SW2','SW1+SW2');

% Example 2 - Different Frequencies
% Declare initial parameters
f = 1; 
a = 1;
phi = 0;
Fs = 100;
t = 0:1/Fs:1;
sw1 = a * sin(2*pi*f.*t+phi);
sw2 = a * sin(2*pi*(f*2).*t+phi); % Change frequency x2
% Addition of arrays in MATLAB is element-wise by default
sw3 = sw1+sw2;
    
figure; % Create a new figure window
plot(t,sw1,'--',t,sw2,'.',t,sw3);
xlabel('Time (sec.)');
ylabel('Amplitude');
title('Addition of 2 Sine Waves - Different Frequencies');
legend('SW1','SW2','SW1+SW2');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_08/additionExample.m -->

<!-- BEGIN_FILE: Ch_08/ampModulation.m -->
#### `Ch_08/ampModulation.m`

````matlab
% AMPMODULATION
% This script provides an example for modulating 
% the amplitude of a carrier signal. This process
% is called Tremolo when used as an audio effect

clear;clc;close all;

% Import Carrier Signal
[carrier,Fs] = audioread('RhythmGuitar.wav');
Ts = 1/Fs; 
N = length(carrier);
t = [0:N-1]*Ts; t = t(:);
plot(t,carrier);title('Original Soundfile (Electric Guitar)');
xlabel('Time (sec.)');figure;

% Tremolo Parameters
depth = 100; % [0,100]
speed = 5;
amp = 0.5*(depth/100);
offset = 1 - amp;

% Synthesize Modulation Signal
f = speed;  % speed of effect
phi = 0;
sw = sin(2*pi*f.*t+phi);

mod = amp.*sw + offset;  

% Plot to compare the original sine wave with the modulator
% Index only first second of signal for visualization purposes
plot(t(1:Fs),sw(1:Fs),t(1:Fs),mod(1:Fs));
title('Modulator Signal');figure;

% Modulate the amplitude of the carrier by the modulator
output = carrier.*mod;

% Plot the output and listen to the result
plot(t(1:Fs),carrier(1:Fs));
title('Carrier Signal (Unprocessed)');
figure;plot(t(1:44100),output(1:44100));
title('Output Signal (Processed)');
sound(output,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_08/ampModulation.m -->

<!-- BEGIN_FILE: Ch_08/equalFades.m -->
#### `Ch_08/equalFades.m`

````matlab
% EQUALFADES
% This script analyzes an exponential crossfade
% for "equal amplitude" or "equal power"

clear;clc;close all;
Fs = 44100;

% Square-root Fades
x = 2; % x can be any number >= 2
numOfSamples = 1*Fs; % 1 second fade in/out
aIn = linspace(0,1,numOfSamples); aIn = aIn(:);
fadeIn = (aIn).^(1/x);

aOut = linspace(1,0,numOfSamples); aOut = aOut(:);
fadeOut = (aOut).^(1/x);

% Compare Amplitude vs. Power of Cross-fade
plot(aIn,fadeIn,aIn,fadeOut,aIn,fadeIn+fadeOut,...
    aIn,(fadeIn.^2) + (fadeOut.^2));
axis([0 1 0 1.5]);
legend('Fade-in','Fade-out','Crossfade Amplitude','Crossfade Power');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_08/equalFades.m -->

<!-- BEGIN_FILE: Ch_08/exponentialFade.m -->
#### `Ch_08/exponentialFade.m`

````matlab
% EXPONENTIALFADE
% This script creates exponential fades. Both
% convex and concave examples are provided.
% These fades are applied to the beginning and end
% of a sine wave test signal.
%
% See also LINEARFADE

clear;clc;close all;
Fs = 48000;
Ts = (1/Fs);
t = 0:Ts:3;
t = t(:);
f = 100; phi = 0;
in = sin(2*pi*f.*t + phi);
figure(1);
plot(t,in);

% Convex Fades 
numOfSamples = 1*Fs; % 1 second fade in/out

% Exponent for curve
x = 2; % "x" can be any number > 1 (linear  = 1)

a = linspace(0,1,numOfSamples); a = a(:);
fadeOut = 1 - a.^x;

a = linspace(1,0,numOfSamples); a = a(:);
fadeIn = 1 - a.^x;

figure(2);
plot(a,fadeIn,a,fadeOut);legend('Fade In','Fade Out');

% Fade In
temp = in;
temp(1:numOfSamples) = fadeIn .* in(1:numOfSamples); 
figure(3);
plot(t,temp);

% Fade Out
out = temp;
out(end-numOfSamples+1:end) = fadeOut.*temp(end-numOfSamples+1:end);
figure(4);
plot(t,out);

% (Alternate) Concave Fades 
a = linspace(0,1,numOfSamples); a = a(:);
fadeOut = a.^x;
a = linspace(1,0,numOfSamples); a = a(:);
fadeIn = a.^x;
figure(5);
plot(a,fadeIn,a,fadeOut);legend('Fade-in','Fade-out');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_08/exponentialFade.m -->

<!-- BEGIN_FILE: Ch_08/linearFade.m -->
#### `Ch_08/linearFade.m`

````matlab
% LINEARFADE
% This script creates linear fades. Then,
% the "fade-in" is applied to the beginning of 
% a sine wave signal. The "fade-out" is applied
% to the end.
%
% See also EXPONENTIALFADE

clc;clear;close all;
Fs = 48000;
Ts = (1/Fs);
t = 0:Ts:3;
t = t(:);
f = 100; phi = 0;
in = sin(2*pi*f.*t + phi);
figure(1);
plot(t,in);
 
numOfSamples = 1*Fs; % 1 second fade in/out
a = linspace(0,1,numOfSamples); a = a(:);
fadeIn = a;
fadeOut = 1-a; % Equivalent = linspace(1,0,numOfSamples);
figure(2);
plot(a,fadeIn,a,fadeOut);legend('Fade-in','Fade-out');

% Fade-In
% Index samples just at the start of the signal
temp = in;
temp(1:numOfSamples) = fadeIn .* in(1:numOfSamples);
figure(3);
plot(t,temp);

% Fade-Out
% Index samples just at the end of the signal
out = temp; 
out(end-numOfSamples+1:end) = fadeOut.*temp(end-numOfSamples+1:end); 
figure(4);
plot(t,out);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_08/linearFade.m -->

<!-- BEGIN_FILE: Ch_08/morphLFO.m -->
#### `Ch_08/morphLFO.m`

````matlab
% MORPHLFO
%
% This script demonstrates the method of morphing
% the LFO signal for the Tremolo effect from
% a triangle wave to a square wave
%
% See also AMPMODULATION

clear; clc; close all;

% Initial Parameters
Fs = 48000; Ts = 1/Fs;
f = 1; % 1 Hz LFO
t = 0:Ts:1; t = t(:);

% Consider a parameter knob with values from 1 - 10
knobValue = 10;
lfo = sawtooth(2*pi*f*t+pi/2,0.5); 
N = length(lfo);
lfoShape = zeros(N,1);
for n = 1:N
   
   if lfo(n,1) >= 0
       % This process is similar to adding an exponent
       % to a linear fade. It turns it into an
       % exponential, convex curve. In this case, it
       % turns the straight line of a triangle wave
       % into a curve closer to a square wave.
       lfoShape(n,1) = lfo(n,1) ^ (1/knobValue);
   else 
       % Need to avoid using negative numbers with power function
       lfoShape(n,1) = -1*abs(lfo(n,1)) ^ (1/knobValue);
   end
end

plot(t,0.5*lfo+0.5,t,0.5*lfoShape+0.5);
legend('Triangle, Knob = 1','Square, Knob = 10');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_08/morphLFO.m -->

<!-- BEGIN_FILE: Ch_08/plottf.m -->
#### `Ch_08/plottf.m`

````matlab
%PLOTTF Plot sampled signal in time and frequency domains
%   PLOTTF(x,Fs) plots the time-domain samples in vector x, assuming that 
%   Fs is an audio sampling rate (44.1k, 48k, etc.) in samples/second, 
%   and also plots the the Fourier transform on the decibel scale
%   between the frequencies of 20 Hz and 20 kHz, logarithmically spaced.
%

function plottf(x,Fs)
Ts = 1/Fs;
N = length(x);
t = [0:N-1]*Ts; t=t(:);

subplot(2,1,1);
plot(t,x); xlabel('Time (sec.)'); ylabel('Amplitude');

% Fourier Transform
len = N;
if len < 4096
    len = 4096;
end
X=(2/N)*fft(x,len);        % do DFT/FFT

f= [0:len-1]*(Fs/len);

% Ensure there will be no values of -Inf dB
% by making the minimum value = -120 dB
X(abs(X)<0.000001) = 0.000001;

subplot(2,1,2);
semilogx(f,20*log10(abs(X))); axis([20 20000 -60 4]);
ax = gca; ax.XTick =[20 50 100 200 500 1000 2000 5000 10000 20000];
xlabel('Frequency (Hz)'); ylabel('Amplitude (dB)');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_08/plottf.m -->

<!-- BEGIN_FILE: Ch_08/ringModulation.m -->
#### `Ch_08/ringModulation.m`

````matlab
% RINGMODULATION
%
% This script demonstrates the result of performing 
% ring modulation with two different frequencies.
%
% By multiplying 300 Hz with 100 Hz, the result
% is a signal with two harmonics: 
% 200 Hz (300 - 100) and 400 Hz (300 + 100)

clc; clear;
% Initial Parameters
Fs = 48000; Ts = 1/Fs;
lenSec = 2;
N = lenSec * Fs;
fHigh = 300;
fLow = 100;

% Synthesize signals and perform element-wise multiplication
x = zeros(N,1);
for n = 1:N
   t = (n-1) * Ts;
   x(n,1) = sin(2*pi*fLow*t) * sin(2*pi*fHigh*t); 
end

plottf(x,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');

    
````
<!-- END_FILE: Ch_08/ringModulation.m -->

<!-- BEGIN_FILE: Ch_08/sCurveFade.m -->
#### `Ch_08/sCurveFade.m`

````matlab
% SCURVEFADE
% This script demonstrates one approach to creating
% s-curve fades. This method involves concatenating
% a convex fade with a concave fade.
%
% See also LINEARFADE, EXPONENTIALFADE, SINECURVEFADE

clear;clc;

Fs = 44100; % Arbitrary Sampling Rate

% S-Curve Fade In 
numOfSamples = round(1*Fs); % 1 sec. fade, round to whole sample
a = linspace(0,1,numOfSamples/2); a = a(:);
x = 2; % x can be any number >= 1
concave = 0.5*(a).^x;
convex = 0.5*(1 - (1-a).^x)+0.5;
fadeIn = [concave ; convex];

% S-Curve Fade Out 
x = 3; % x can be any number >= 1
convex = 0.5*(1 - a.^x)+0.5;
concave = 0.5*(1-a).^x;
fadeOut = [convex ; concave];

% Plot the S-Curves
t = linspace(0,1,numOfSamples); t=t(:);
plot(t,fadeIn,t,fadeOut); legend('Fade-in','Fade-out');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_08/sCurveFade.m -->

<!-- BEGIN_FILE: Ch_08/sineCurveFade.m -->
#### `Ch_08/sineCurveFade.m`

````matlab
% SINECURVEFADE
% This script demonstrates one approach to creating
% s-curve fades. This method involves using
% the sine function.
%
% See also LINEARFADE, EXPONENTIALFADE, SCURVEFADE

clear;clc;

Fs = 44100; % Arbitrary Sampling Rate
Ts = 1/Fs;
% S-Curve Fade In 
lenSec = 1; % 1 second fade in/out
N = round(lenSec * Fs);   % Convert to whole # of samples
t = [0:N-1]*Ts; t = t(:);

% The s-curve fade is half a cycle of a sine wave
% If fade is 1 sec., period of sine wave is 2 sec.
period = 2 * N * Ts;  % units of seconds
freq = 1/period;      % units of Hz
fadeIn = 0.5*sin(2*pi*freq*t - pi/2) + 0.5;

% S-Curve Fade Out 
fadeOut = 0.5*sin(2*pi*freq*t + pi/2) + 0.5;

% Plot the S-Curves
plot(t,fadeIn,t,fadeOut); legend('Fade-in','Fade-out');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_08/sineCurveFade.m -->

<!-- BEGIN_FILE: Ch_08/subtractionExample.m -->
#### `Ch_08/subtractionExample.m`

````matlab
% SUBTRACTIONEXAMPLE
% This script provides two examples for combining signals 
% together using subtraction
%
% This first example is for signals of the same frequency and phase
%
% The second example shows the addition of signals where 
% one signal has a phase offset of 180 degrees (pi radians)
%
% See also ADDITIONEXAMPLE

% Example 1 - Same Frequency and Phase
% Declare initial parameters
f = 1;  a = 1; phi = 0; Fs = 100;
t = 0:1/Fs:1;
sw1 = a * sin(2*pi*f.*t+phi); sw2 = a * sin(2*pi*f.*t+phi);
% Element-wise subtraction
sw3 = sw1 - sw2;
% Plot the result
plot(t,sw1,'.',t,sw2,t,sw3,'--');
xlabel('Time (sec.)');
ylabel('Amplitude');
title('Subtraction of 2 Sine Waves - Same Frequency and Phase');
legend('SW1','SW2','SW1-SW2');

% Example 2 - Same frequency with a phase offset

sw1 = a * sin(2*pi*f.*t+0);
sw2 = a * sin(2*pi*f*t+pi); % Phase offset by 180 degrees

sw3 = sw1+sw2;
    
figure; % Create a new figure window
plot(t,sw1,'.',t,sw2,t,sw3,'--');
xlabel('Time (sec.)');
ylabel('Amplitude');
title('Addition of 2 Sine Waves - 180 degree phase offset');
legend('SW1','SW2','SW1-SW2');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_08/subtractionExample.m -->

<!-- END_CHAPTER: Ch_08 -->

<!-- BEGIN_CHAPTER: Ch_09 -->
## Ch_09

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_09/autoPanExample.m` | 40 | 1255 |
| code | `Ch_09/goniometer.m` | 92 | 2438 |
| code | `Ch_09/goniometerExample.m` | 52 | 1319 |
| code | `Ch_09/midSideProcessing.m` | 45 | 1205 |
| code | `Ch_09/pan.m` | 51 | 1537 |
| code | `Ch_09/stereoImager.m` | 52 | 1415 |

### Source Files

<!-- BEGIN_FILE: Ch_09/autoPanExample.m -->
#### `Ch_09/autoPanExample.m`

````matlab
% AUTOPANEXAMPLE
% This script implements the Automatic Panning (auto-pan)
% effect. The function "pan" is used to process an
% input signal, along with an array of pan values
% for each sample number in the signal
%
% See also PAN
clc; clear;

% Import test sound file
[in,Fs] = audioread('RhythmGuitar.wav'); 
N = length(in); Ts = 1/Fs;
% Create time vector t of samples for LFO
t = [0:N-1]*Ts; t = t(:);
f = 1; % Frequency of panning LFO

% Generate sine wave LFO based on f and t
panValue = 100*sin(2*pi*f*t);
panType = 2; % Start with panType = 2, but try 1,2,3

% Run pan function
[out] = pan(in,panValue,panType);

sound(out,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_09/autoPanExample.m -->

<!-- BEGIN_FILE: Ch_09/goniometer.m -->
#### `Ch_09/goniometer.m`

````matlab
% GONIOMETER
%
% This function analyzes a stereo audio signal and
% creates a goniometer plot. This visualization indicates
% the stereo width of a signal. 
%
% Values along the vertical axis represent parts of 
% the signal in the middle (or center) of the 
% stereo field. This occurs when the left and right 
% channels are identical. Conversely, values along 
% the horizontal axis represent parts of the signal 
% when the left and right channels have opposite polarities. 
%
% Values at an angle of 45 degrees represent when 
% there is a signal panned to the right channel and 
% the left channel has zero amplitude. Similarly, values 
% at an angle of 135 degrees represent when there is 
% a signal panned to the left channel and the right 
% channel has zero amplitude.
%
% See also GONIOMETEREXAMPLE
function goniometer(in)
N = length(in);
x = zeros(N,1);
y = zeros(N,1);

for n = 1:N
    L = in(n,1);
    R = in(n,2);
    
    radius = sqrt(L^2 + R^2);
    angle = atan2(L,R);
    angle = angle + (pi/4); % Rotate by convention
    
    x(n,1) = radius * cos(angle);
    y(n,1) = radius * sin(angle);

end

line([-1 1] ,[-1 1],'Color',[0.75 0.75 0.75]);
line([-1 1] ,[1 -1],'Color',[0.75 0.75 0.75]);
line([0 0] ,[-1 0.95],'Color',[0.75 0.75 0.75]);
line([-0.95 1] ,[0 0],'Color',[0.75 0.75 0.75]);
hold on;
% Circle
th = 0:pi/50:2*pi;
xunit = cos(th);
yunit = sin(th);
plot(xunit, yunit,'Color',[0.75 0.75 0.75]);
% Left
xL = -0.75;
yL = 0.8;
txtL = 'L';
text(xL,yL,txtL,'Color',[0.75 0.75 0.75]);

%Right
xR = 0.73;
yR = 0.8;
txtR = 'R';
text(xR,yR,txtR,'Color',[0.75 0.75 0.75]);

%Mid
xM = -0.018;
yM = 0.96;
txtM = 'M';
text(xM,yM,txtM,'Color',[0.75 0.75 0.75]);

%Side
xS = -0.98;
yS = 0;
txtS = 'S';
text(xS,yS,txtS,'Color',[0.75 0.75 0.75]);

% Plot Data
plot(x,y,'.b'); axis([-1 1 -1 1]);
hold off;

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_09/goniometer.m -->

<!-- BEGIN_FILE: Ch_09/goniometerExample.m -->
#### `Ch_09/goniometerExample.m`

````matlab
% GONIOMETEREXAMPLE
%
% This script demonstrates several example plots
% of the goniometer. 
%
% Examples include a signal panned to the center,
% left, and right. Finally, an example is shown
% for when the left and right channels have opposite
% polarity.
%
% See also GONIOMETER

% Test Signal
Fs = 48000; Ts = 1/Fs;
f = 10; t = [0:Ts:.1].';
x = sin(2*pi*f*t);

% Center
panCenter = [0.707*x , 0.707*x];
subplot(2,2,1);
goniometer(panCenter);

% Left
panLeft = [x , zeros(size(x))];
subplot(2,2,2);
goniometer(panLeft);

% Right
panRight = [zeros(size(x)),x];
subplot(2,2,3);
goniometer(panRight);

% Opposite Polarities
polarity = [0.707 * x , 0.707 * (-x)];
subplot(2,2,4);
goniometer(polarity);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_09/goniometerExample.m -->

<!-- BEGIN_FILE: Ch_09/midSideProcessing.m -->
#### `Ch_09/midSideProcessing.m`

````matlab
% MIDSIDEPROCESSING
% This script performs mid-side (sum and difference)
% encoding and decoding. 
clc; clear;

[input,Fs] = audioread('stereoDrums.wav');

% Separate stereo signal into two mono signals
left = input(:,1);
right = input(:,2);

% Mid-side Encoding
mid = 0.5 * (left + right);
sides = 0.5 * (left - right);

% Add additional processing here 
% (e.g. distortion, compression, etc.)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Mid-side Decoding
newL = mid + sides;
newR = mid - sides;

output = [newL , newR];

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_09/midSideProcessing.m -->

<!-- BEGIN_FILE: Ch_09/pan.m -->
#### `Ch_09/pan.m`

````matlab
% PAN
% This function pans a mono audio signal in a stereo field.
% It is implemented such that it can pan the entire signal
% to one location if "panValue" is a scalar. It can also be
% used for auto-pan effects if "panValue" is an array.
%
% Input Variables
%   panType : 1=Linear, 2=sqRt, 3 = sine-law
%   panValue : (-100 to +100) transformed to a scale of (0-1)

function [out] = pan(in, panValue, panType)

% Convert Pan Value to a Normalized Scale
panTransform = (panValue/200) + 0.5;

%conditional statements determining panType
if panType == 1
    leftAmp = 1-panTransform;
    rightAmp = panTransform;
elseif panType == 2
    leftAmp = sqrt(1-panTransform);
    rightAmp = sqrt(panTransform);
elseif panType == 3
    leftAmp = sin((1-panTransform) * (pi/2));
    rightAmp = sin(panTransform * (pi/2));
    
end

leftChannel = leftAmp.*in;
rightChannel = rightAmp.*in;

out = [leftChannel,rightChannel];

end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_09/pan.m -->

<!-- BEGIN_FILE: Ch_09/stereoImager.m -->
#### `Ch_09/stereoImager.m`

````matlab
% STEREOIMAGER
% This script demonstrates the stereo image widening effect. 
% The effect is based on mid-side processing. The parameter 
% "width" can be used to make the example drums file sound
% wider or narrower.
%
% See also MIDSIDEPROCESSING

clc; clear;
[in,Fs] =audioread('stereoDrums.wav');

% Splitting signal into right and left channels
L = in(:,1);
R = in(:,2);

% Create Mid and Side Channels
side = 0.5 * (L-R);
mid = 0.5 * (L+R);

% Width Amount (wider if > 1, narrower if < 1)
width = 1.5;

% Scale the mid/side with width
sideNew = width .* side;
midNew = (2 - width) .* mid;

% Create new M/S signal
newLeft = midNew + sideNew;
newRight = midNew - sideNew;

% Combine Signals, Concatenated side-by-side, 2 columns
out = [newLeft , newRight];

% Play the sound 
sound(out,Fs) 

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_09/stereoImager.m -->

<!-- END_CHAPTER: Ch_09 -->

<!-- BEGIN_CHAPTER: Ch_10 -->
## Ch_10

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_10/arctanDistortion.m` | 37 | 1080 |
| code | `Ch_10/asymmetrical.m` | 45 | 1277 |
| code | `Ch_10/bitReduct.m` | 51 | 1652 |
| code | `Ch_10/cubicDistortion.m` | 37 | 1091 |
| code | `Ch_10/diode.m` | 38 | 1058 |
| code | `Ch_10/distortionExample.m` | 96 | 2389 |
| code | `Ch_10/exponential.m` | 37 | 1095 |
| code | `Ch_10/fullwaveRectification.m` | 40 | 1159 |
| code | `Ch_10/halfwaveRectification.m` | 40 | 1152 |
| code | `Ch_10/hardClip.m` | 45 | 1277 |
| code | `Ch_10/infiniteClip.m` | 43 | 1313 |
| code | `Ch_10/parallelDistortion.m` | 46 | 1228 |
| code | `Ch_10/piecewise.m` | 37 | 1155 |
| code | `Ch_10/thdExample.m` | 38 | 1105 |

### Source Files

<!-- BEGIN_FILE: Ch_10/arctanDistortion.m -->
#### `Ch_10/arctanDistortion.m`

````matlab
% ARCTANDISTORTION
% This function implements arctangent soft-clipping
% distortion. An input parameter "alpha" is used 
% to control the amount of distortion applied 
% to the input signal.
%
% Input variables
%   in : input signal
%   alpha : drive amount (1-10)
%
% See also CUBICDISTORTION, DISTORTIONEXAMPLE

function [out] = arctanDistortion(in,alpha)

N = length(in);
out = zeros(N,1);
for n = 1:N
    
   out(n,1) = (2/pi)*atan(in(n,1)*alpha); 
    
end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_10/arctanDistortion.m -->

<!-- BEGIN_FILE: Ch_10/asymmetrical.m -->
#### `Ch_10/asymmetrical.m`

````matlab
% ASYMMETRICAL
% This function creates a distortion effect
% which is neither "even" or "odd". Therefore,
% the resulting signal has both even and 
% odd harmonics.
%
% Input variables
%   in : input signal
%   dc : offset amount
%
% See also CUBICDISTORTION, DISTORTIONEXAMPLE

function out = asymmetrical(in,dc)
N = length(in);
x = in + dc; % Introduce DC Offset
y = zeros(N,1);

for n = 1:N
    % Conditional to ensure "out" is a 
    % monotonically increasing function
    if abs(x(n,1)) > 1
        x(n,1) = sign(x(n,1));
    end
    
    % Non-linear, distortion function
    y(n,1) = x(n,1) - (1/5)*x(n,1)^5;
    
end
out = y - dc; % Remove DC Offset

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_10/asymmetrical.m -->

<!-- BEGIN_FILE: Ch_10/bitReduct.m -->
#### `Ch_10/bitReduct.m`

````matlab
% BITREDUCT
% This function creates a bit reduction or
% bit crushing distortion. It uses an input
% variable, "nBits", to determine the number of 
% amplitude values in the output signal. This 
% algorithm can have a fractional number of bits,
% similar to the processing found in some audio plug-ins.
%
% Input variables
%   inputSignal : array of samples for the input signal
%   nBits : scalar for the number of desired bits
%
% See also DISTORTIONEXAMPLE, ROUND, CEIL, FLOOR, FIX

function [out] = bitReduct(in,nBits)

% Determine the desired number of possible amplitude values
ampValues = 2 ^ nBits;

% Shrink the full-scale signal (-1 to 1, peak-to-peak)
% to fit within a range of 0 to 1
prepInput = 0.5*in + 0.5;

% Scale the signal to fit within the range of the possible values
scaleInput = ampValues * prepInput;

% Round the signal to the nearest integers
roundInput = round(scaleInput);

% Invert the scaling to fit the original range
prepOut = roundInput / ampValues;

% Fit in full-scale range 
out = 2*prepOut - 1;

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_10/bitReduct.m -->

<!-- BEGIN_FILE: Ch_10/cubicDistortion.m -->
#### `Ch_10/cubicDistortion.m`

````matlab
% CUBICDISTORTION
% This function implements cubic soft-clipping
% distortion. An input parameter "a" is used 
% to control the amount of distortion applied 
% to the input signal.
%
% Input variables
%   in : input signal
%   a : drive amount (0-1), amplitude of 3rd harmonic
%
% See also ARCTANDISTORTION, DISTORTIONEXAMPLE

function [ out ] = cubicDistortion(in,a)

N = length(in);
out = zeros(N,1);
for n = 1:N
    
    out(n,1) = in(n,1) - a*(1/3)*in(n,1)^3;
    
end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_10/cubicDistortion.m -->

<!-- BEGIN_FILE: Ch_10/diode.m -->
#### `Ch_10/diode.m`

````matlab
% DIODE
% This function implements the Shockley
% Ideal Diode Equation for audio signals
% with an amplitude between -1 to 1 FS
%
% See also ASYMMETRICAL, DISTORTIONEXAMPLE

function [out] = diode(in)

% Diode Characteristics
Vt = 0.0253; % thermal voltage
eta = 1.68; % emission coefficient
Is = .105;  % saturation current

N = length(in);
out = zeros(N,1);

for n = 1:N
    
    out(n,1) = Is * (exp(0.1*in(n,1)/(eta*Vt)) - 1);
    
end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_10/diode.m -->

<!-- BEGIN_FILE: Ch_10/distortionExample.m -->
#### `Ch_10/distortionExample.m`

````matlab
% DISTORTIONEXAMPLE
% This script is used to test various distortion functions.
% Each algorithm can be analyzed by "uncommenting" the
% code under each section. The waveform, characteristic curve,
% and total-harmonic distortion (THD) is plotted for 
% each function.
%
% See also THDEXAMPLE

clear;clc;

Fs = 48000; Ts = 1/Fs;
f = 2;       % f = 2 (Waveform and Char Curve), f = 2500 (THD)
t = [0:Ts:1].';
in = sin(2*pi*f*t);  % Used as input signal for each distortion

%%% Infinite Clipping
%out = infiniteClip(in);

%%% Half-wave Rectification
%out = halfwaveRectification(in);

%%% Full-wave Rectification
%out = fullwaveRectification(in);

%%% Hard-clipping
%thresh = 0.5;
%out = hardClip(in,thresh);

%%% Cubic soft-clipping
%a = 1;  % Amount: 0 (no distortion) - 1 (full)
%out = cubicDistortion(in,a);

%%% Arctangent Distortion
%alpha = 5;
%out = arctanDistortion(in,alpha);

%%% Sine Distortion
%out = sin((pi/2)*in);

%%% Exponential Soft-clipping
%G = 4;
%out = exponential(in,G);

%%% Piece-wise Overdrive
%out = piecewise(in);

% Diode Clipping
%out = diode(in);

% Asymmetrical Distortion
%dc = -0.25;
%out = asymmetrical(in,dc);

%%% Bit Crushing
%nBits = 8;
%out = bitReduct(in,nBits);
    
%%% Dither noise
dither = 0.02*randn(size(in));
nBits = 4;
out = bitReduct(in+dither,nBits);

%%% Plotting
figure(1);   % Use f = 2 (above)
subplot(1,2,1); % Waveform
plot(t,in,t,out); axis([0 1 -1.1 1.1]);
xlabel('Time (sec.)');ylabel('Amplitude');
title('Waveform');

subplot(1,2,2); % Characteristic Curve
plot(in,in,in,out); axis([-1 1 -1.1 1.1]);
xlabel('Input Amplitude'); ylabel('Output Amplitude');
legend('Linear','Distortion'); title('Characteristic Curve');


figure(2); % Total Harmonic Distortion Plot (f = 2500)
thd(out,Fs,5);
axis([0 24 -50 0]);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_10/distortionExample.m -->

<!-- BEGIN_FILE: Ch_10/exponential.m -->
#### `Ch_10/exponential.m`

````matlab
% EXPONENTIAL
% This function implements exponential soft-clipping
% distortion. An input parameter "G" is used 
% to control the amount of distortion applied 
% to the input signal.
%
% Input variables
%   in : input signal
%   G : drive amount (1-10)
%
% See also CUBICDISTORTION, ARCTANDISTORTION, DISTORTIONEXAMPLE

function [ out ] = exponential(in,G)

N = length(in);
out = zeros(N,1);
for n = 1:N
    
    out(n,1) = sign(in(n,1)) * (1 - exp(-abs(G*in(n,1))));
    
end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_10/exponential.m -->

<!-- BEGIN_FILE: Ch_10/fullwaveRectification.m -->
#### `Ch_10/fullwaveRectification.m`

````matlab
% FULLWAVERECTIFICATION
% This function implements full-wave rectification
% distortion. Amplitude values of the input signal
% which are negative are changed to positive in the
% output signal.
%
% See also HALFWAVERECTIFICATION, DISTORTIONEXAMPLE

function [ out ] = fullwaveRectification(in)

N = length(in);
out = zeros(N,1);
for n = 1:N
   
    if in(n,1) >= 0 
        % If positive, assign input to output
        out(n,1) = in(n,1);
    else
        % If negative, flip input
        out(n,1) = -1*in(n,1);
        
    end
    
end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_10/fullwaveRectification.m -->

<!-- BEGIN_FILE: Ch_10/halfwaveRectification.m -->
#### `Ch_10/halfwaveRectification.m`

````matlab
% HALFWAVERECTIFICATION
% This function implements full-wave rectification
% distortion. Amplitude values of the input signal
% which are negative are changed to zero in the
% output signal.
%
% See also FULLWAVERECTIFICATION, DISTORTIONEXAMPLE

function [out] = halfwaveRectification(in)

N = length(in);
out = zeros(N,1);
for n = 1:N
   
    if in(n,1) >= 0 
        % If positive, assign input to output
        out(n,1) = in(n,1);
    else
        % If negative, set output to zero
        out(n,1) = 0;
        
    end
    
end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_10/halfwaveRectification.m -->

<!-- BEGIN_FILE: Ch_10/hardClip.m -->
#### `Ch_10/hardClip.m`

````matlab
% HARDCLIP
% This function implements hard-clipping
% distortion. Amplitude values of the input signal
% which are greater than a threshold are clipped.
%
% Input Variables
%   in : signal to be processed
%   thresh : maximum amplitude where clipping occurs
%
% See also INFINITECLIP, PIECEWISE, DISTORTIONEXAMPLE

function [out] = hardClip(in,thresh)

N = length(in);
out = zeros(N,1);
for n = 1:N
   
    if in(n,1) >= thresh 
        % If true, assign output = thresh
        out(n,1) = thresh;
    elseif in(n,1) <= -thresh
        % If true, set output = -thresh
        out(n,1) = -thresh;
    else
        out(n,1) = in(n,1);
        
    end
    
end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_10/hardClip.m -->

<!-- BEGIN_FILE: Ch_10/infiniteClip.m -->
#### `Ch_10/infiniteClip.m`

````matlab
% INFINITECLIP
% This function implements infinite clipping
% distortion. Amplitude values of the input signal
% which are positive are changed to 1 in the
% output signal. Amplitude values of the input signal
% which are negative are changed to -1 in the
% output signal.
%
% See also HARDCLIP, DISTORTIONEXAMPLE

function [out] = infiniteClip(in)

N = length(in);
out = zeros(N,1);
for n = 1:N
    % Change all amplitude values to +1 or -1 (FS amplitude)
    % "Pin the Rails" (description in audio electronics)
    if in(n,1) >= 0 
        % If positive, assign output = 1
        out(n,1) = 1;
    else
        % If negative, set output = -1
        out(n,1) = -1;
        
    end
    
end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_10/infiniteClip.m -->

<!-- BEGIN_FILE: Ch_10/parallelDistortion.m -->
#### `Ch_10/parallelDistortion.m`

````matlab
% PARALLELDISTORTION
% This script demonstrates how to create parallel
% distortion. It allows for the "dry" unprocessed
% signal to be blended with the "wet" processed 
% signal.
%
% See also ARCTANDISTORTION

clear; clc;close all;

[in,Fs] = audioread('AcGtr.wav');

% Alpha - Amount of distortion
alpha = 8;

% Wet Path - Distortion
dist = arctanDistortion(in,alpha);

% Pick an arbitrary "wet/dry mix" value 
mix = 50;  % Experiment with values from 0 - 100

% Convert to a linear gain value
g = mix/100; 
 
% Add together "Wet/Dry" signals
out = g * dist + (1-g) * in;

% Listen to Result
sound(out,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_10/parallelDistortion.m -->

<!-- BEGIN_FILE: Ch_10/piecewise.m -->
#### `Ch_10/piecewise.m`

````matlab
% PIECEWISE
% This function implements a piecewise distortion
% algorithm. Within one operating region, the 
% input signal is not distorted. When the signal
% is outside of that operating region, it is clipped.
%
% See also HARDCLIP, DISTORTIONEXAMPLE

function [ out ] = piecewise(in)

out = zeros(size(in));

for n = 1:length(in)
    if abs(in(n,1)) <= 1/3
        out(n,1) = 2*in(n,1);
    elseif abs(in(n,1)) > 2/3 
        out(n,1) = sign(in(n,1));
    else
        out(n,1) = sign(in(n,1))*(3-(2-3*abs(in(n,1)))^2)/3;
    end
end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_10/piecewise.m -->

<!-- BEGIN_FILE: Ch_10/thdExample.m -->
#### `Ch_10/thdExample.m`

````matlab
% THDEXAMPLE
% This script demonstrates how to analyze and 
% visualize total harmonic distortion (THD)
% for a non-linear function.
%
% See also DISTORTIONEXAMPLE

clear; clc;
Fs = 48000; Ts = 1/Fs; Nyq = Fs/2;
t = [0:Ts:1-Ts];
f = 2500;
% Input signal
x = sin(2*pi*f*t);
% Hypothetical distortion effect outputs a square wave
y = square(2*pi*f*t);

% Use THD function
thd(y,Fs); % Opens Plot
r = thd(y,Fs);  % Returns THD in dB

% Convert dB to percentage
percentTHD = 10^(r/20) * 100

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_10/thdExample.m -->

<!-- END_CHAPTER: Ch_10 -->

<!-- BEGIN_CHAPTER: Ch_11 -->
## Ch_11

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_11/convertSecSamples.m` | 41 | 1271 |
| code | `Ch_11/convertTempoSamples.m` | 48 | 1475 |
| code | `Ch_11/convolutionExample.m` | 59 | 1722 |
| code | `Ch_11/echoFeedback.m` | 66 | 1847 |
| code | `Ch_11/echoSync.m` | 67 | 1878 |
| code | `Ch_11/impFIR.m` | 62 | 1743 |
| code | `Ch_11/impIIR.m` | 54 | 1673 |
| code | `Ch_11/reverbConv.m` | 39 | 1179 |
| asset | `Ch_11/reverbIR.wav` | - | 1152044 |

### Source Files

<!-- BEGIN_FILE: Ch_11/convertSecSamples.m -->
#### `Ch_11/convertSecSamples.m`

````matlab
% CONVERTSECSAMPLES
% This script provides two examples for converting a time delay
% in units of seconds to samples and milliseconds to samples
%
% See also CONVERTTEMPOSAMPLES

% Example 1 - Seconds to Samples

Fs = 48000; % arbitrary sampling rate 

timeSec = 1.5; % arbitrary time in units of seconds

% Convert to units of samples
timeSamples = fix(timeSec * Fs); % round to nearest integer sample


% Example 2 - Milliseconds to Samples

timeMS = 330; % arbitrary time in units of milliseconds

% Convert to units of seconds
timeSec = timeMS/1000; 

% Convert to units of samples
timeSamples = fix(timeSec * Fs); % round to nearest integer sample

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_11/convertSecSamples.m -->

<!-- BEGIN_FILE: Ch_11/convertTempoSamples.m -->
#### `Ch_11/convertTempoSamples.m`

````matlab
% CONVERTTEMPOSAMPLES
% This script provides an examples for calculating a delay time 
% in units of samples which will be synchronized with the tempo
% of a song in units of beats per minutes (BPM).
%
% Assume a (4/4) time signature where a BEAT = QUARTER NOTE
%
% See also CONVERTSECSAMPLES

% Example - Convert Tempo Sync'd Delay to Samples

Fs = 48000; % arbitrary sampling rate 

beatsPerMin = 90; % arbitrary tempo in units of beats per minute

% Calculate Beats Per Second
beatsPerSec = beatsPerMin / 60; % 1 minute / 60 seconds

% Calculate # of Seconds Per Beat
secPerBeat = 1/beatsPerSec;

% Note Division
% 4 = whole, 2 = half, 1 = quarter, 0.5 = 8th, 0.25 = 16th
noteDiv = 1 ; 

% Calculate Delay Time in Seconds
timeSec = noteDiv * secPerBeat;

% Convert to Units of Samples
timeSamples = fix(timeSec * Fs); % round to nearest integer sample

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_11/convertTempoSamples.m -->

<!-- BEGIN_FILE: Ch_11/convolutionExample.m -->
#### `Ch_11/convolutionExample.m`

````matlab
% convolutionExample.m
% This script demonstrates the MATLAB Convolution
% function - 'y = conv(x,h)'
% The example demonstrated is with a single cycle of a 
% sine wave. When the sine wave is convolved with the
% impulse response for an echo effect, the output
% signal has delayed copies of the sine wave at 
% different amplitudes at different times.
%
% See also CONV

clear; clc; close all;
% Import Previously Saved IR
[h,Fs] = audioread('impResp.wav');

% Synthesize input signal
x = zeros(2*Fs,1); % 2 second long input signal
f = 4;
t = [0:Fs*0.25 - 1]/Fs;
x(1:Fs*0.25) = sin(2*pi*f*t);

% Perform Convolution
y = conv(x,h);

% Plot Signals
xAxis = (0:length(h)-1)/Fs;
subplot(3,1,1);
plot(xAxis,x); % Plot the Impulse Response
axis([-0.1 2 -1.1 1.1]);
xlabel('Time (sec.)');
title('Input Signal - x[n]');

subplot(3,1,2);
stem(xAxis,h); % Plot the Impulse Response
axis([-0.1 2 -0.1 1.1]);
xlabel('Time (sec.)');
title('Impulse Response - h[n]');

subplot(3,1,3);
plot(xAxis,y(1:2*Fs)); % Plot the Impulse Response
axis([-0.1 2 -1.1 1.1]);
xlabel('Time (sec.)');
title('Output Signal - y[n]');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_11/convolutionExample.m -->

<!-- BEGIN_FILE: Ch_11/echoFeedback.m -->
#### `Ch_11/echoFeedback.m`

````matlab
% ECHOFEEDBACK
% This script demonstrates one example to create a
% feed-back, tempo-synchronized echo effect
%
% See also ECHOSYNC

% Import our audio file
[x,Fs] = audioread('AcGtr.wav');

% Known tempo of recording
beatsPerMin = 102;  % units of beats per minute

% Calculate Beats Per Second
beatsPerSec = beatsPerMin / 60; % 1 minute / 60 seconds

% Calculate # of Seconds Per Beat
secPerBeat = 1/beatsPerSec;

% Note Division
% 4 = whole, 2 = half, 1 = quarter, 0.5 = 8th, 0.25 = 16th
noteDiv = 0.5 ; 

% Calculate Delay Time in Seconds
timeSec = noteDiv * secPerBeat;

% Convert to Units of Samples
d = fix(timeSec * Fs); % round to nearest integer sample

a = -0.75; % amplitude of delay branch

% Index each element of our signal to create the output
N = length(x);
y = zeros(N,1);
for n = 1:N
    
    % When the sample number is less than the time delay
    % Avoid indexing a negative sample number
    if n < d+1
        % output = input
        y(n,1) = x(n,1);
     
    % Now add in the delayed signal   
    else
        % output = input + delayed version of output
        % reduce relative amplitude of delay to 3/4
        y(n,1) = x(n,1) + (-a)*y(n-d,1);    
    end
end

sound(y,Fs); % Listen to the effect

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_11/echoFeedback.m -->

<!-- BEGIN_FILE: Ch_11/echoSync.m -->
#### `Ch_11/echoSync.m`

````matlab
% ECHOSYNC
% This script demonstrates one example to create a
% feed-forward, tempo-synchronized echo effect
%
% See also CONVERTTEMPOSAMPLES

% Import our audio file
[x,Fs] = audioread('AcGtr.wav');

% Known tempo of recording
beatsPerMin = 102;  % units of beats per minute

% Calculate Beats Per Second
beatsPerSec = beatsPerMin / 60; % 1 minute / 60 seconds

% Calculate # of Seconds Per Beat
secPerBeat = 1/beatsPerSec;

% Note Division
% 4 = whole, 2 = half, 1 = quarter, 0.5 = 8th, 0.25 = 16th
noteDiv = 0.5 ; 

% Calculate Delay Time in Seconds
timeSec = noteDiv * secPerBeat;

% Convert to Units of Samples
d = fix(timeSec * Fs); % round to nearest integer sample

b = 0.75; % amplitude of delay branch

% Total number of samples
N = length(x);
y = zeros(N,1);
% Index each element of our signal to create the output
for n = 1:N
    
    % When the sample number is less than the time delay
    % Avoid indexing a negative sample number
    if n < d+1
        % output = input
        y(n,1) = x(n,1);
     
    % Now add in the delayed signal   
    else
        % output = input + delayed version of input
        % reduce relative amplitude of delay to 3/4
        y(n,1) = x(n,1) + b*x(n-d,1);    
    end
end

sound(y,Fs); % Listen to the effect

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_11/echoSync.m -->

<!-- BEGIN_FILE: Ch_11/impFIR.m -->
#### `Ch_11/impFIR.m`

````matlab
% IMPFIR
% This script demonstrates one example to measure
% the impulse response of an FIR system 
%
% See also IMPIIR

clear;clc;close all;
Fs = 48000;
N = Fs*2;
% Synthesize the impulse signal
imp = zeros(N,1); % 2 second long signal of 0's
imp(1,1) = 1;  % Change the first sample = 1

d1 = 0.5*Fs;  % 1/2 second delay
b1 = 0.7; % Gain of first delay line

d2 = 1.5*Fs;  % 3/2 second delay
b2 = 0.5;     % Gain of second delay line

% Zero-pad the beginning of the signal for indexing
% based on the maximum delay time
pad = zeros(d2,1);
impPad = [pad;imp];

out = zeros(N,1);
% Index each element of our signal to create the output
for n = 1:N
    index= n+d2;
    out(n,1) = impPad(index,1) ...
        + b1*impPad(index-d1,1) + b2*impPad(index-d2,1);
end

t = [0:N-1]/Fs;
subplot(1,2,1);
stem(t,imp); % Plot the Impulse Response
axis([-0.1 2 -0.1 1.1]);
xlabel('Time (sec.)');
title('Input Impulse');
subplot(1,2,2);
stem(t,out); % Plot the Impulse Response
axis([-0.1 2 -0.1 1.1]);
xlabel('Time (sec.)');
title('Output Impulse Response');

% Save the Impulse Response for Future Use
audiowrite('impResp.wav',out,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_11/impFIR.m -->

<!-- BEGIN_FILE: Ch_11/impIIR.m -->
#### `Ch_11/impIIR.m`

````matlab
% IMPFIR
% This script demonstrates one example to approximate
% the impulse response of an IIR system 
%
% See also IMPFIR

clear;clc;close all;
Fs = 48000; Ts = 1/Fs;
N = Fs*2;   % Number of samples
% Synthesize the impulse signal
imp = zeros(N,1); % 2 second long signal of 0's
imp(1,1) = 1;  % Change the first sample = 1

d1 = 0.5*Fs;  % 1/2 second delay
a1 = -0.7; % Gain of feed-back delay line

% Index each element of our signal to create the output
for n = 1:d1
    out(n,1) = imp(n,1); % Initially there is no delay
end
for n = d1+1:Fs*2        % Then there is signal + delay
    out(n,1) = imp(n,1) + a1*out(n-d1);
end
for n = Fs*2+1:Fs*10     % Finally there is only delay 
    out(n,1) = a1*out(n-d1);  % After input finished
end
t = [0:N-1]*Ts;
subplot(1,2,1);
stem(t,imp); % Plot the Impulse Response
axis([-0.1 2 -0.1 1.1]);
xlabel('Time (sec.)');
title('Input Impulse');
subplot(1,2,2);
t = [0:length(out)-1]*Ts;
stem(t,out); % Plot the Impulse Response
axis([-0.1 10 -1.1 1.1]);
xlabel('Time (sec.)');
title('Output Impulse Response');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_11/impIIR.m -->

<!-- BEGIN_FILE: Ch_11/reverbConv.m -->
#### `Ch_11/reverbConv.m`

````matlab
% REVERBCONV
% This script demonstrates the process to create 
% a stereo convolution reverb by using a two-channel
% impulse response. This impulse response is based 
% on a measurement of a recording studio 
% in Nashville, Tennessee.

clear;clc;close all;

% Import sound file and IR measurement
[x,Fs] = audioread('AcGtr.wav'); % Mono signal
[h] = audioread('reverbIR.wav'); % Stereo IR

% Visualize one channel of the impulse response
plot(h(:,1));

% Perform Convolution
yLeft = conv(x,h(:,1));
yRight = conv(x,h(:,2));

y = [yLeft,yRight];

sound(y,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_11/reverbConv.m -->

<!-- END_CHAPTER: Ch_11 -->

<!-- BEGIN_CHAPTER: Ch_12 -->
## Ch_12

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_12/bandStopFilter.m` | 45 | 1247 |
| code | `Ch_12/convolutionFiltering.m` | 41 | 1231 |
| code | `Ch_12/fir1Example.m` | 53 | 1420 |
| code | `Ch_12/fir2Example.m` | 34 | 1024 |
| code | `Ch_12/pinkNoise1.m` | 64 | 1793 |

### Source Files

<!-- BEGIN_FILE: Ch_12/bandStopFilter.m -->
#### `Ch_12/bandStopFilter.m`

````matlab
% BANDSTOPFILTER
% This script creates a band-stop filter by performing
% parallel processing with a LPF and HPF
%
% See also FIR1

% Design Filters
% Note: W_lpf must be less than W_hpf to 
% create a band-stop filter
order = 24;
W_lpf = 0.25;    % Normalized freq of LPF
lpf = fir1(order,W_lpf);

W_hpf = 0.75;    % Normalized freq of HPF
hpf = fir1(order,W_hpf,'high');

% Impulse Input Signal
input = [1, 0];

% Separately, find impulse response of LPF and HPF
u = conv(input,lpf);

w = conv(input,hpf);

% Create combined parallel output by adding together IRs
output = u + w; 

% Plot the frequency response
freqz(output);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_12/bandStopFilter.m -->

<!-- BEGIN_FILE: Ch_12/convolutionFiltering.m -->
#### `Ch_12/convolutionFiltering.m`

````matlab
% convolutionFiltering.m
%
% This script demonstrates how to use a built-in, FIR
% filter design function to create the impulse response
% for a LPF. Then, the filtering is performed on an
% audio signal using the convolution operation.


% Import our audio file
[x,Fs] = audioread('AcGtr.wav');
Nyq = Fs/2;

n = 30; % Order of the Filter

freqHz = 500; % frequency in Hz
Wn = freqHz/Nyq; % Normalized frequency for fir1

[ h ] = fir1(n,Wn);  % Filter design function

% "h" is the impulse response of the filter

% Convolution applies the filter to a signal
y = conv(x,h);  

sound(y,Fs); % Listen to the effect

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_12/convolutionFiltering.m -->

<!-- BEGIN_FILE: Ch_12/fir1Example.m -->
#### `Ch_12/fir1Example.m`

````matlab
% FIR1EXAMPLE
% This script demonstrates the various uses of 
% the 'fir1' filter function
%
% See also FIR1

% Declare initial parameters for LPF and HPF
n = 12; % order of filter = # of delay lines + 1
f = 6000; % cut-off frequency (Hz)
Fs = 48000; nyq = Fs/2;
Wn = f/nyq;

% Syntax for low-pass filter
h_lpf = fir1(n,Wn); % optionally, h = fir1(n,Wn,'low'); 

% Syntax for high-pass filter
h_hpf = fir1(n,Wn,'high'); 


% Declare second frequency for BPF and BSF
f2 = 18000;
Wn2 = f2/nyq;


% Syntax for band-pass filter
h_bpf = fir1(n,[Wn Wn2]); 

% Syntax for band-stop filter
h_bsf = fir1(n,[Wn Wn2],'stop'); 


% Plots
figure(1); freqz(h_lpf); % Low-pass filter
figure(2); freqz(h_hpf); % High-pass filter
figure(3); freqz(h_bpf); % Band-pass filter
figure(4); freqz(h_bsf); % Band-stop filter

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_12/fir1Example.m -->

<!-- BEGIN_FILE: Ch_12/fir2Example.m -->
#### `Ch_12/fir2Example.m`

````matlab
% FIR2EXAMPLE
% This script demonstrates the syntax of
% the 'fir2' function to create a filter with
% arbitrary frequency response
%
% See also FIR2

% Declare function parameters
n = 30;   % filter order
frqs = [0, 0.2, 0.5,0.8,1];    % normalized frequencies
amps = [2, 4, 0.25,2,1];  % linear amplitudes for each freq


% Syntax for function
h = fir2(n,frqs,amps);

% Plot Frequency Response
freqz(h);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_12/fir2Example.m -->

<!-- BEGIN_FILE: Ch_12/pinkNoise1.m -->
#### `Ch_12/pinkNoise1.m`

````matlab
% PINKNOISE1
% This script synthesizes an approximation of pink noise
% using an FIR filter.
%
% Pink noise can be created by filtering white noise. The
% amplitude response of the filter has a gain of 1/sqrt(f),
% where "f" is frequency (Hz).
%
% See also FIR2

clc; clear; 
Fs = 48000;     % Sampling rate
Nyq = Fs/2;     % Nyquist Frequency for Normalization
sec = 5;        % 5 seconds of noise
white = randn(sec*Fs,1);


f = 20;    % Starting frequency in Hz
gain = 1/sqrt(f);  % Amplitude at starting frequency

freq = 0;  % Initialize fir2 freq vector  
while f < Nyq
    % Normalized Frequency Vector
    freq = [freq f/Nyq];
    
    % Amplitude vector, gain = 1/sqrt(f)
    gain = [gain 1/sqrt(f)];
    
    % Increase "f" by an octave
    f = f*2;
end
% Set frequency and amplitude at nyquist
freq = [freq 1];
gain = [gain 1/sqrt(Nyq)];

% Filter Normalization Factor to Unity Gain
unity = sqrt(20);
gain = unity * gain;

% Plot Frequency Reponse of Filter
order = 2000;
h = fir2(order,freq,gain);
[H,F] = freqz(h,1,4096,Fs);
semilogx(F,20*log10(abs(H))); axis([20 20000 -30 0]);

% Create Pink Noise by Filtering White Noise
pink = conv(white,h);
sound(pink,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_12/pinkNoise1.m -->

<!-- END_CHAPTER: Ch_12 -->

<!-- BEGIN_CHAPTER: Ch_13 -->
## Ch_13

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_13/basicFilterbank.m` | 42 | 1242 |
| code | `Ch_13/biquadExample.m` | 44 | 1284 |
| code | `Ch_13/biquadFilter.m` | 192 | 4989 |
| code | `Ch_13/filterbankExample.m` | 49 | 1228 |
| code | `Ch_13/filterExample.m` | 39 | 1102 |
| code | `Ch_13/impzExample.m` | 42 | 1139 |
| code | `Ch_13/lufs.m` | 78 | 2089 |
| code | `Ch_13/lufsExample.m` | 53 | 1794 |
| code | `Ch_13/pinkNoise2.m` | 39 | 1323 |
| code | `Ch_13/slewRateDistortion.m` | 51 | 1647 |
| code | `Ch_13/slewRateExample.m` | 39 | 1271 |
| asset | `Ch_13/1kHz Sine -40 LUFS-16bit.wav` | - | 3840044 |
| asset | `Ch_13/AcGtr.wav` | - | 631248 |
| asset | `Ch_13/AcGtr_1.wav` | - | 2734660 |
| asset | `Ch_13/EBU-reference_listening_signal_pinknoise_500Hz_2kHz_R128.wav` | - | 11520044 |
| asset | `Ch_13/RhythmGuitar.wav` | - | 1981070 |
| asset | `Ch_13/stereoDrums.wav` | - | 3758308 |

### Source Files

<!-- BEGIN_FILE: Ch_13/basicFilterbank.m -->
#### `Ch_13/basicFilterbank.m`

````matlab
% BASICFILTERBANK
% This script creates a two-band filterbank using
% a LPF and HPF. The butterworth filter design
% function is used to create the LPF and HPF,
% both with the same cut-off frequency.
% The magnitude response of each filter is 
% plotted together.
% 
% See also BUTTER, FREQZ

Fs = 48000;
Nyq = Fs/2;
n = 8;
Wn = 1000/Nyq;

[bLow,aLow] = butter(n,Wn);
[bHi,aHi] = butter(n,Wn,'high');

[hLow,w] = freqz(bLow,aLow,4096,Fs);
[hHi] = freqz(bHi,aHi,4096,Fs);

semilogx(w,20*log10(abs(hLow)),w,20*log10(abs(hHi)));
axis([20 20000 -24 6]);
xlabel('Frequency (Hz)');
ylabel('Amplitude (dB)');
legend('LPF','HPF');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_13/basicFilterbank.m -->

<!-- BEGIN_FILE: Ch_13/biquadExample.m -->
#### `Ch_13/biquadExample.m`

````matlab
% BIQUADEXAMPLE
% This script demonstrates the use of the 
% bi-quad filter function. Various filter
% types and topologies can be tested.
%
% See also BIQUADFILTER

% Impulse Response of Bi-quad
x = [1;zeros(4095,1)];

% Filter Parameters
Fs = 48000;
f = 1000;    % Frequency in Hz
Q = 0.707;
dBGain = -6;

% FILTER TYPE >>> lpf,hpf,pkf,apf,nch,hsf,lsf,bp1,bp2
type = 'lpf'; 
% TOPOLOGY >>> 1 = Direct Form I, 2 = II, 3 = Transposed II
form = 3; 

y = biquadFilter(x,Fs,f,Q,dBGain,type,form);

% Plot Amplitude Response of Filter
[h,w] = freqz(y,1,4096,Fs);
semilogx(w,20*log10(abs(h)));axis([20 20000 -20 15]);
xlabel('Frequency (Hz)'); ylabel('Amplitude (dB)');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_13/biquadExample.m -->

<!-- BEGIN_FILE: Ch_13/biquadFilter.m -->
#### `Ch_13/biquadFilter.m`

````matlab
% BIQUADFILTER
% This function implements a bi-quad filter based
% on the Audio EQ Cookbook Coefficients. All filter
% types can be specified (LPF, HPF, BPF, etc.) and
% three different topologies are included.
%
% Input Variables
%   f0 : filter frequency (cut-off or center based on filter)
%   Q : bandwidth parameter 
%   dBGain : gain value on the decibel scale
%   type : 'lpf','hpf','pkf','bp1','bp2','apf','lsf','hsf'
%   form : 1 (Direct Form I), 2 (DFII), 3 (Transposed DFII)

function [out] = biquadFilter(in,Fs,f0,Q,dBGain,type,form)

%%% Initial Parameters
N = length(in);
out = zeros(length(in),1);

%%% Intermediate Variables
%
w0 = 2*pi*f0/Fs;            % Angular Freq. (Radians/sample) 
alpha = sin(w0)/(2*Q);      % Filter Width
A  = sqrt(10^(dBGain/20));  % Amplitude

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% TYPE - LPF,HPF,BPF,APF,HSF,LSF,PKF,NCH
%
%----------------------
%        LPF
%----------------------
if strcmp(type,'lpf')
    b0 =  (1 - cos(w0))/2;
    b1 =   1 - cos(w0);
    b2 =  (1 - cos(w0))/2;
    a0 =   1 + alpha;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha;

%----------------------
%        HPF
%----------------------
elseif strcmp(type,'hpf')
    b0 =  (1 + cos(w0))/2;
    b1 = -(1 + cos(w0));
    b2 =  (1 + cos(w0))/2;
    a0 =   1 + alpha;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha;

%----------------------
%   Peaking Filter
%----------------------
elseif strcmp(type,'pkf')
    b0 =   1 + alpha*A;
    b1 =  -2*cos(w0);
    b2 =   1 - alpha*A;
    a0 =   1 + alpha/A;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha/A;

%----------------------
%   Band-pass Filter 1
%----------------------
% Constant skirt gain, peak gain = Q
elseif strcmp(type,'bp1')
    b0 =   sin(w0)/2;
    b1 =   0;
    b2 =  -sin(w0)/2;
    a0 =   1 + alpha;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha;

%----------------------
%   Band-pass Filter 2
%----------------------
% Constant 0 dB peak gain
elseif strcmp(type,'bp2')
    b0 =   alpha;
    b1 =   0;
    b2 =  -alpha;
    a0 =   1 + alpha;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha;

%----------------------
%    Notch Filter
%----------------------
elseif strcmp(type,'nch')
    b0 =   1;
    b1 =  -2*cos(w0);
    b2 =   1;
    a0 =   1 + alpha;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha;        

%----------------------
%    All-Pass Filter
%----------------------
elseif strcmp(type,'apf')
    b0 =   1 - alpha;
    b1 =  -2*cos(w0);
    b2 =   1 + alpha;
    a0 =   1 + alpha;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha;

%----------------------
%    Low-Shelf Filter
%----------------------
elseif strcmp(type,'lsf')
    b0 = A*((A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha);
    b1 = 2*A*((A-1) - (A+1)*cos(w0));
    b2 = A*((A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha);
    a0 = (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha;
    a1 = -2*((A-1) + (A+1)*cos(w0));
    a2 = (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha;

%----------------------
%    High-Shelf Filter
%----------------------
elseif strcmp(type,'hsf')
    b0 = A*( (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha);
    b1 = -2*A*((A-1) + (A+1)*cos(w0));
    b2 = A*((A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha);
    a0 = (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha;
    a1 = 2*((A-1) - (A+1)*cos(w0));
    a2 = (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha;

% Otherwise, no filter
else 
    b0 = 1; a0 = 1;
    b1 = 0; b2 = 0; a1 = 0; a2 = 0;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Topology - Direct Form I, II, Transposed II
if (form == 1) % Direct Form I
    x2 = 0;    % Initial conditions 
    x1 = 0;
    y2 = 0;
    y1 = 0;
    for n = 1:N
        out(n,1) = (b0/a0)*in(n,1) + (b1/a0)*x1 + (b2/a0)*x2 ...
            + (-a1/a0)*y1 + (-a2/a0)*y2;
        x2 = x1;
        x1 = in(n,1);
        y2 = y1;
        y1 = out(n,1);
    end

elseif (form == 2) % Direct Form II
   w1 = 0;     % w1 & w2 are delayed versions of 'w'
   w2 = 0;
   for n = 1:N
       w = in(n,1) + (-a1/a0)*w1 + (-a2/a0)*w2;  
       out(n,1) = (b0/a0)*w + (b1/a0)*w1 + (b2/a0)*w2;
       w2 = w1;
       w1 = w;
   end

elseif (form == 3) % Transposed Direct Form II
   d1 = 0;    % d1 & d2 are outputs of the delay blocks
   d2 = 0;
   for n = 1:N
       out(n,1) = (b0/a0)*in(n,1) + d1;
       d1 = (b1/a0)*in(n,1) + (-a1/a0)*out(n,1) + d2;
       d2 = (b2/a0)*in(n,1) + (-a2/a0)*out(n,1); 
   end
   
else % No filtering  
   
    out = in;
    
end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_13/biquadFilter.m -->

<!-- BEGIN_FILE: Ch_13/filterbankExample.m -->
#### `Ch_13/filterbankExample.m`

````matlab
% FILTERBANKEXAMPLE
% This script creates a two-band filterbank using
% a LPF and HPF
%
% See also BUTTER

clear; clc;

Fs = 48000;
Nyq = Fs/2;
m = 2; % Filter Order

numOfBands = 4;

% Logarithmically spaced cut-off frequencies
% 2*10^1 - 2*10^4 (20-20k) Hz
freq = 2 * logspace(1,4,numOfBands+1);

for band = 1:numOfBands
   
    Wn = [freq(band) , freq(band+1)] ./ Nyq;
    [b(:,band),a(:,band)] = butter(m,Wn);
    
    [h,w] = freqz(b(:,band),a(:,band),4096,Fs);
    semilogx(w,20*log10(abs(h)));
    hold on;
    
end

hold off;
axis([20 20000 -24 6]);
xlabel('Frequency (Hz)');
ylabel('Amplitude (dB)');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_13/filterbankExample.m -->

<!-- BEGIN_FILE: Ch_13/filterExample.m -->
#### `Ch_13/filterExample.m`

````matlab
% FILTEREXAMPLE
%
% This script demonstrates how to use a built-in, IIR
% filter design function to create the impulse response
% for a LPF. Then, the filtering is performed on an
% audio signal using the "filter" function.
%
% See also BUTTER, FILTER

% Import our audio file
[x,Fs] = audioread('AcGtr_1.wav');
Nyq = Fs/2;

m = 4; % Order of the Filter

freqHz = 500; % frequency in Hz
Wn = freqHz/Nyq;

[b,a] = butter(m,Wn);

y = filter(b,a,x);

sound(y,Fs); % Listen to the effect

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_13/filterExample.m -->

<!-- BEGIN_FILE: Ch_13/impzExample.m -->
#### `Ch_13/impzExample.m`

````matlab
% IMPZEXAMPLE
% This script demonstrates how to use a built-in function
% "impz" to approximate an IIR system as an FIR system.
% Then, filtering is performed on an audio
% signal using the convolution operation.
%
% See also IMPZ, CONV, BUTTER

% Import our audio file
[x,Fs] = audioread('AcGtr_1.wav');
Nyq = Fs/2;

m = 4; % Order of the Filter

freqHz = 2000; % frequency in Hz
Wn = freqHz/Nyq;

[b,a] = butter(m,Wn);

h = impz(b,a); % Approximate System

y = conv(x,h);

sound(y,Fs); % Listen to the effect

stem(h);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_13/impzExample.m -->

<!-- BEGIN_FILE: Ch_13/lufs.m -->
#### `Ch_13/lufs.m`

````matlab
% LUFS
% This function calculates the loudness of a mono or stereo
% audio signal based on the LUFS/LKFS standard. The analysis
% involves multiple steps. First, the input signal is
% processed using the pre-filter followed by the RLB filter.
% Then a mean-square calculation is performed. Finally,
% all the channels are summed together and loudness is 
% converted to units of decibels (dB).
%
% See also LUFSEXAMPLE

function [loudness] = lufs(x)
% Number of samples
N = length(x);
% Determine whether mono or stereo
numOfChannels = size(x,2);

% Initialize Pre-filter
b0 = 1.53512485958697;
a1 = -1.69065929318241;
b1 = -2.69169618940638;
a2 = 0.73248077421585;
b2 = 1.19839281085285;
a0 = 1;

b = [b0, b1,b2];
a = [a0,a1,a2];

% Perform Pre-filtering
w = zeros(size(x));
for channel = 1:numOfChannels % Loop in case it is stereo
    w(:,channel) = filter(b,a,x(:,channel));
end

% RLB Filter
b0 = 1.0;
a1 = -1.99004745483398; 
b1 = -2.0;
a2 = 0.99007225036621;
b2 = 1.0;
a0 = 1;
b = [b0, b1,b2];
a = [a0,a1,a2];


% Perform RLB Filtering
y = zeros(size(x));
for channel = 1:numOfChannels
    y(:,channel) = filter(b,a,w(:,channel));
end


% Perform Mean-square Amplitude Analysis
z = zeros(1,numOfChannels);
for channel = 1:numOfChannels
    % Add together the square of the samples, 
    % then divide by the number of samples
    z(1,channel) = sum(y(:,channel).^2)/N;
end

% Determine loudness (dB) by summing all channels
loudness = -0.691 + 10 * log10(sum(z));

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_13/lufs.m -->

<!-- BEGIN_FILE: Ch_13/lufsExample.m -->
#### `Ch_13/lufsExample.m`

````matlab
% LUFSEXAMPLE
% This script demonstrates the use of the lufs
% function for calculating loudness based
% on the LUFS/LKFS standard. Four examples
% are shown. First, a mono recording of
% an electric guitar is analyzed. Second,
% a stereo recording of drums is analyzed.
% 
% Then, two more examples are demonstrated using
% test signals provided by the European Broadcast
% Union (EBU) for the sake of verifying proper 
% measurement. A filtered pink noise signal 
% is measured with a loudness of -23 LUFS. Finally,
% a sine wave signal is measured with a loudness 
% of -40 LUFS.
%
% See also LUFS
clc;clear;

% Example 1 - Mono Electric Guitar
sig1 = audioread('RhythmGuitar.wav');
loudnessGuitar = lufs(sig1)

% Example 2 - Stereo Drums
sig2 = audioread('stereoDrums.wav');
loudnessDrums = lufs(sig2)

% Example 3 - Filtered noise signal provide by EBU for verification
fnm='EBU-reference_listening_signal_pinknoise_500Hz_2kHz_R128.wav';
sig3 = audioread(fnm);
loudnessNoise = lufs(sig3)  % Should equal -23 LUFS

% Example 3 - Sinewave provide by EBU for verification
sig4 = audioread('1kHz Sine -40 LUFS-16bit.wav');
loudnessSine = lufs(sig4) % Should equal -40 LUFS

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_13/lufsExample.m -->

<!-- BEGIN_FILE: Ch_13/pinkNoise2.m -->
#### `Ch_13/pinkNoise2.m`

````matlab
% PINKNOISE2
% This script synthesizes an approximation of pink noise
% using an IIR filter.
%
% Pink noise can be created by filtering white noise. The
% amplitude response of the filter decreases by 10 dB/decade
% or ~3 dB/octave.

clc; clear; close all;
Fs = 48000;     % Sampling rate
Nyq = Fs/2;     % Nyquist Frequency for Normalization
sec = 5;        % 5 seconds of noise
white = randn(sec*Fs,1);

% IIR Coefficients
b = [0.049922035, -0.095993537, 0.050612699, -0.004408786];
a = [1 -2.494956002   2.017265875  -0.522189400];
[H,F] = freqz(b,a,4096,Fs);
semilogx(F,20*log10(abs(H))); axis([20 20000 -30 0]);

% Create Pink Noise by Filtering White Noise
pink = filter(b,a,white);
sound(pink,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_13/pinkNoise2.m -->

<!-- BEGIN_FILE: Ch_13/slewRateDistortion.m -->
#### `Ch_13/slewRateDistortion.m`

````matlab
% SLEWRATEDISTORTION
% This function implements slew-rate distortion. Frequencies
% greater than the "maxFreq" parameter are distorted.
% Frequencies less than "maxFreq" are not distorted. This type
% of distortion occurs in op-amps used for audio.
% 
% Input Variables
%   in : input signal to be processed
%   Fs : sampling rate
%   maxFreq : the limiting/highest frequency before distortion

function [out] = slewRateDistortion(in,Fs,maxFreq)

Ts = 1/Fs;
peak = 1;
slewRate = maxFreq*2*pi*peak; % Convert freq to slew rate

slope = slewRate*Ts; % Convert slew rate to slope/sample

out = zeros(size(in));      % Total number of samples
prevOut = 0;         % Initialize feedback delay sample
for n = 1:length(in)
    
    % Determine the change between samples
    dlta = in(n,1) - prevOut;
    if dlta > slope  % Don't let dlta exceed max slope 
        dlta = slope;
    elseif dlta < -slope
        dlta = -slope;
    end
    
    out(n,1) = prevOut + dlta;
    prevOut = out(n,1); % Save current "out" for next loop 
end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_13/slewRateDistortion.m -->

<!-- BEGIN_FILE: Ch_13/slewRateExample.m -->
#### `Ch_13/slewRateExample.m`

````matlab
% SLEWRATEEXAMPLE
% This script demonstrates how to use the 
% slewRateDistortion function. Two examples are
% provided, with a sine wave and with a square wave.
%
% See also SLEWRATEDISTORTION

% Initial Parameters
Fs = 48000; Ts = 1/Fs;
f = 5;   % Low frequency for sake of plotting
t = [0:Ts:1].';

x = sin(2*pi*f*t);    % Example 1: Sine Wave
%x = square(2*pi*f*t); % Example 2: Square Wave

maxFreq = 3;  % 
% Note: if maxFreq >= "f" of sine wave, then no distortion
% If maxFreq < "f" of input, then slew rate distortion
y = slewRateDistortion(x,Fs,maxFreq);

plot(t,x);hold on;
plot(t,y);hold off; axis([0 1 -1.1 1.1]);
legend('Input','Output');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_13/slewRateExample.m -->

<!-- END_CHAPTER: Ch_13 -->

<!-- BEGIN_CHAPTER: Ch_14 -->
## Ch_14

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_14/circularBuffer.m` | 21 | 701 |
| code | `Ch_14/circularBufferExample.m` | 37 | 1049 |
| code | `Ch_14/cubicInterpolationDelay.m` | 47 | 1637 |
| code | `Ch_14/delayBufferExample.m` | 72 | 1758 |
| code | `Ch_14/feedbackDelay.m` | 18 | 476 |
| code | `Ch_14/feedbackDelayExample.m` | 39 | 840 |
| code | `Ch_14/linearInterpolationDelay.m` | 29 | 900 |
| code | `Ch_14/moduloOperator.m` | 14 | 254 |
| code | `Ch_14/simpleLinearBuffer.m` | 48 | 1660 |

### Source Files

<!-- BEGIN_FILE: Ch_14/circularBuffer.m -->
#### `Ch_14/circularBuffer.m`

````matlab
% CIRCULARBUFFER
% This function performs series delay and uses
% a circular buffer. Rather than shifting
% all the values in the array buffer during each
% iteration, the index changes each time through
% based on the current sample, "n"
%
% Additional input variables
%    delay : samples of delay
%    n : current sample number used for circular buffer

function [out,buffer] = circularBuffer(in,buffer,delay,n)

% Determine indexes for circular buffer
len = length(buffer);
indexC = mod(n-1,len) + 1; % Current index in circular buffer
indexD = mod(n-delay-1,len) + 1; % Delay index in circular buffer

out = buffer(indexD,1);
% Store the current output in appropriate index
buffer(indexC,1) = in;
````
<!-- END_FILE: Ch_14/circularBuffer.m -->

<!-- BEGIN_FILE: Ch_14/circularBufferExample.m -->
#### `Ch_14/circularBufferExample.m`

````matlab
% CIRCULARBUFFEREXAMPLE
% This script tests a circular buffer function and 
% demonstrates how it works
%
% Execution of the script is set to "pause" during each iteration
% to allow a user to view the contents of the delay buffer
% during each step. To advance from one step to the next,
% press <ENTER>
%
% See also CIRCULARBUFFER

clear;clc;
in = [1 ; -1 ; 2 ; -2 ; 3 ; zeros(5,1)];
buffer = zeros(6,1); 

% Number of samples of delay 
delay = 4; 

N = length(in);
out = zeros(N,1);
% Series Delay
for n = 1:N

    [out(n,1),buffer] = circularBuffer(in(n,1),buffer,delay,n);
    % Display current status values
    disp(['The current sample number is: ' , ...
        num2str(n)]);
    disp(['The current buffer index is: ' , ...
        num2str(mod(n-1,6) + 1)]);
    disp(['The current delay index is: ' , ...
        num2str(mod(n-delay-1,6) + 1)]);
    disp(['The input is: ' , num2str(in(n,1))]);
    disp(['The delay buffer is: [' , num2str(buffer.') , ']']);
    disp(['The output is: ' , num2str(out(n,1))]); disp(' ');
    pause;
  
end
````
<!-- END_FILE: Ch_14/circularBufferExample.m -->

<!-- BEGIN_FILE: Ch_14/cubicInterpolationDelay.m -->
#### `Ch_14/cubicInterpolationDelay.m`

````matlab
% CUBICINTERPOLATIONDELAY
% This script demonstrates how to introduce a
% fractional (non-integer) delay. Cubic interpolation
% is used to estimate an amplitude value in-between
% adjacent samples.

clear;clc;
in = [1 , zeros(1,9)]; % Horizontal for displaying in command window

fracDelay = 3.2;             % Fractional delay length in samples
intDelay = floor(fracDelay); % Round down to get the previous (3)
frac = fracDelay - intDelay; % Find the fractional amount (0.2)

buffer = zeros(1,5); % length(buffer) >= ceil(fracDelay)+1

N = length(in);
out = zeros(1,N);
% Series Fractional Delay
for n = 1:N
    % Calculate intermediate variable for cubic interpolation
    a0 = buffer(1,intDelay+2) - buffer(1,intDelay+1) - ...
        buffer(1,intDelay-1) + buffer(1,intDelay);
    
    a1 = buffer(1,intDelay-1) - buffer(1,intDelay) - a0;
    
    a2 = buffer(1,intDelay+1) - buffer(1,intDelay-1);
    
    a3 = buffer(1,intDelay);
    
    % 
    out(1,n) = a0*(frac^3) + a1*(frac^2) + a2*frac + a3;
    
    buffer = [in(1,n)  buffer(1,1:end-1)];    % Shift buffer
end

% Compare the input & output signals
disp(['The orig. input signal was: ', num2str(in)]);
disp(['The final output signal is: ', num2str(out)]);

plot(out);
% Observe in this plot, the impulse at sample n=1 is delayed
% by 3.2 samples. Therefore, the output signal should have
% an impulse at time 4.2 samples. With cubic interpoloation
% this impulse contributes to the amplitude of the output 
% signal at samples 3,4,5,6. The result of cubic interpolation
% is a closer approximation to the underlying (smooth) 
% continuous signal than linear interpolation.
````
<!-- END_FILE: Ch_14/cubicInterpolationDelay.m -->

<!-- BEGIN_FILE: Ch_14/delayBufferExample.m -->
#### `Ch_14/delayBufferExample.m`

````matlab
% DELAYBUFFEREXAMPLES
% This script demonstrates several examples of creating different 
% types (FIR, IIR) of systems by using a delay buffer
%
% Execution of the script is set to "pause" during each iteration
% to allow a user to view the contents of the delay buffer
% during each step. To advance from one step to the next,
% press <ENTER>
%
% See also SIMPLELINEARBUFFER
clear;clc;
in = [1, -1, 2 , -2, zeros(1,6)];

buffer = zeros(1,20); % Longer buffer than delay length

% Number of samples of delay 
delay = 5; % Does not need to be the same length as buffer

N = length(in);
out = zeros(1,N);
% Series Delay
for n = 1:N
    
    out(1,n) = buffer(1,delay);
    buffer = [in(1,n) , buffer(1,1:end-1)];
    
end

% Compare the input & output signals
disp('Series Delay: 5 Samples');
disp('out(n) = in(n-5)');
disp(['The orig. input signal was: ', num2str(in)]);
disp(['The final output signal is: ', num2str(out)]);
pause; clc; % Press <ENTER> to continue


% Feed-forward (FIR) System
buffer = zeros(1,20);
delay = 3; % Number of samples of delay 

% Parallel Delay Line
for n = 1:N
    
    out(1,n) = in(1,n) + buffer(1,delay);
    buffer = [in(1,n) , buffer(1,1:end-1)];
    
end

disp('Feed-forward Delay: 3 samples');
disp('out(n) = in(n) + in(n-3)');
disp(['The orig. input signal was: ', num2str(in)]);
disp(['The final output signal is: ', num2str(out)]);
pause; clc; % Press <ENTER> to continue


% Feed-back (IIR) System
buffer = zeros(1,20);
for n = 1:N
    
    out(1,n) = in(1,n) + buffer(1,delay);
    buffer = [out(1,n) , buffer(1,1:end-1)];
    
end

disp('Feed-back Delay: 3 samples');
disp('out(n) = in(n) + out(n-3)');
disp(['The orig. input signal was: ', num2str(in)]);
disp(['The final output signal is: ', num2str(out)]);
````
<!-- END_FILE: Ch_14/delayBufferExample.m -->

<!-- BEGIN_FILE: Ch_14/feedbackDelay.m -->
#### `Ch_14/feedbackDelay.m`

````matlab
% FEEDBACKDELAY
% This function performs feed-back delay by processing an
% individual input sample and updating a delay buffer
% used in a loop to index each sample in a signal
%
% Additional input variables
%       delay : samples of delay
%       fbGain : feed-back gain (linear scale)

function [out,buffer] = feedbackDelay(in,buffer,delay,fbGain)

out = in + fbGain*buffer(delay,1);

% Store the current output in appropriate index
buffer = [out ; buffer(1:end-1,1)];
````
<!-- END_FILE: Ch_14/feedbackDelay.m -->

<!-- BEGIN_FILE: Ch_14/feedbackDelayExample.m -->
#### `Ch_14/feedbackDelayExample.m`

````matlab
% FEEDBACKDELAYEXAMPLE
% This script calls the feed-back delay function and passes in 
% the delay buffer
%
% See also FEEDBACKDELAY

clear;clc;
in = [1 ; -1 ; 2 ; -2 ; zeros(6,1)]; % Input signal

% Longer buffer than delay length to demonstrate
% delay doesn't just have to be the "end" of buffer
buffer = zeros(20,1); 

% Number of samples of delay 
delay = 5; 

% Feed-back Gain Coefficient
fbGain = 0.5;

% Initialize Output Vector
N = length(in);
out = zeros(N,1);

% Series Delay
for n = 1:N
    % Pass "buffer" into feedbackDelay function
    [out(n,1),buffer] = feedbackDelay(in(n,1),buffer,delay,fbGain);
    % Return updated "buffer" for next loop iteration
end
% Print and Compare Input and Output Signals
disp('Feed-back Delay: 5 samples');
disp('The orig. input signal was: ')
in
disp('The final output signal is: ');
out
````
<!-- END_FILE: Ch_14/feedbackDelayExample.m -->

<!-- BEGIN_FILE: Ch_14/linearInterpolationDelay.m -->
#### `Ch_14/linearInterpolationDelay.m`

````matlab
% LINEARINTERPOLATIONDELAY
% This script demonstrates how to introduce a
% fractional (non-integer) delay. Linear interpolation
% is used to estimate an amplitude value in-between
% adjacent samples.

clear;clc;
in = [1 , zeros(1,9)]; % Horizontal for displaying in command window

fracDelay = 3.2;             % Fractional delay length in samples
intDelay = floor(fracDelay); % Round down to get the previous (3)
frac = fracDelay - intDelay; % Find the fractional amount (0.2)

buffer = zeros(1,5); % length(buffer) >= ceil(fracDelay)

N = length(in);
out = zeros(1,N);
% Series Fractional Delay
for n = 1:N
    
    out(1,n) = (1-frac) * buffer(1,intDelay) + ...
        (frac) * buffer(1,intDelay+1);
    
    buffer = [in(1,n)  buffer(1,1:end-1)];
end

% Compare the input & output signals
disp(['The orig. input signal was: ', num2str(in)]);
disp(['The final output signal is: ', num2str(out)]);
````
<!-- END_FILE: Ch_14/linearInterpolationDelay.m -->

<!-- BEGIN_FILE: Ch_14/moduloOperator.m -->
#### `Ch_14/moduloOperator.m`

````matlab
% MODULOOPERATOR
% This script demonstrates the result of the modulo
% operator - mod(a,m);
%
% See also MOD

clear;clc;
m = 4;
for a = 1:15
    
   disp(['When "a" is : ', num2str(a)]); 
   disp(['mod(a,4) is : ', num2str(mod(a,m))]); disp(' ');
   
end
````
<!-- END_FILE: Ch_14/moduloOperator.m -->

<!-- BEGIN_FILE: Ch_14/simpleLinearBuffer.m -->
#### `Ch_14/simpleLinearBuffer.m`

````matlab
% SIMPLELINEARBUFFER
% This script demonstrates the basics of creating a linear buffer.
% An input signal is processed by a loop to index the
% "current" sample and store it in the delay buffer. Each time
% through the loop the delay buffer is shifted to make room
% for a new sample. The output of the process is determined
% by indexing an element at the end of the delay buffer.
%
% Execution of the script is set to "pause" during each iteration
% to allow a user to view the contents of the delay buffer
% during each step. To advance from one step to the next,
% press <ENTER>
%
% See also DELAYBUFFEREXAMPLE
clear;clc;
% Horizontal for displaying in Command Window
in = [1, -1, 2 , -2, zeros(1,6)]; 

% Buffer should be initialized without any value
% Length of buffer = 5, Output is indexed from end of buffer
% Therefore, a delay of 5 samples is created
buffer = zeros(1,5);

N = length(in);
out = zeros(1,N);
for n = 1:N
    
    % Read the output at the current time sample
    % from the end of the delay buffer
    out(1,n) = buffer(1,end);
    disp(['For sample ', num2str(n),' the output is: ', ...
        num2str(out(1,n))]);
    
    % Shift each value in the buffer by one element
    % to make room for the current sample to be stored
    % in the first element
    buffer = [in(1,n)  buffer(1,1:end-1)];
    disp(['For sample ', num2str(n),' the buffer is: ', ...
        num2str(buffer)]);
    
    % Press <ENTER> in the Command Window to continue
    pause;
    disp([' ']); % Blank line
end

% Compare the input & output signals
disp(['The orig. input signal was: ', num2str(in)]);
disp(['The final output signal is: ', num2str(out)]);
````
<!-- END_FILE: Ch_14/simpleLinearBuffer.m -->

<!-- END_CHAPTER: Ch_14 -->

<!-- BEGIN_CHAPTER: Ch_15 -->
## Ch_15

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_15/allPassFilter.m` | 26 | 535 |
| code | `Ch_15/audioSpecGram.m` | 30 | 758 |
| code | `Ch_15/autoWahExample.m` | 48 | 1088 |
| code | `Ch_15/barberpole2Example.m` | 68 | 1663 |
| code | `Ch_15/barberpoleExample.m` | 67 | 1543 |
| code | `Ch_15/barberpoleFlanger.m` | 40 | 1179 |
| code | `Ch_15/barberpoleFlanger2.m` | 64 | 1905 |
| code | `Ch_15/basicPitch.m` | 46 | 1224 |
| code | `Ch_15/basicPitchDown.m` | 44 | 1277 |
| code | `Ch_15/basicPitchUp.m` | 37 | 1011 |
| code | `Ch_15/biquadPhaser.m` | 47 | 1156 |
| code | `Ch_15/biquadWah.m` | 48 | 1134 |
| code | `Ch_15/chorusEffect.m` | 39 | 1124 |
| code | `Ch_15/chorusExample.m` | 31 | 656 |
| code | `Ch_15/crossfades.m` | 49 | 1086 |
| code | `Ch_15/feedbackFlanger.m` | 39 | 1125 |
| code | `Ch_15/flangerEffect.m` | 38 | 1085 |
| code | `Ch_15/flangerExample.m` | 46 | 1204 |
| code | `Ch_15/fractionalDelay.m` | 12 | 397 |
| code | `Ch_15/harmonyExample.m` | 19 | 499 |
| code | `Ch_15/lfoPitch.m` | 101 | 2984 |
| code | `Ch_15/phaserEffect.m` | 40 | 1021 |
| code | `Ch_15/phaserExample.m` | 45 | 1033 |
| code | `Ch_15/phaserExample2.m` | 51 | 1115 |
| code | `Ch_15/pitchShifter.m` | 146 | 4538 |
| code | `Ch_15/pitchShifterExample.m` | 20 | 524 |
| code | `Ch_15/plottf.m` | 33 | 946 |
| code | `Ch_15/vibratoEffect.m` | 36 | 1052 |
| code | `Ch_15/vibratoExample.m` | 34 | 766 |

### Source Files

<!-- BEGIN_FILE: Ch_15/allPassFilter.m -->
#### `Ch_15/allPassFilter.m`

````matlab
% ALLPASSFILTER
% This script demonstrates an implementation of an
% all-pass filter using a delay buffer (Direct Form II)

clear;clc;
in = [1; zeros(100,1)];

buffer = zeros(5,1); % Longer buffer than delay length

% Number of samples of delay 
delay = 2; % Does not need to be the same length as buffer

g = 0.5;

N = length(in);
out = zeros(N,1);
% Series Delay
for n = 1:N
    
    % Series All-pass Filters
    out(n,1) = g*in(n,1) + buffer(delay,1);
    buffer = [in(n,1) + -g*out(n,1) ; buffer(1:end-1,1)];
    
end

freqz(out);
````
<!-- END_FILE: Ch_15/allPassFilter.m -->

<!-- BEGIN_FILE: Ch_15/audioSpecGram.m -->
#### `Ch_15/audioSpecGram.m`

````matlab
% AUDIOSPECGRAM
% This script displays a spectrogram of an audio signal
% using the built-in MATLAB function
%
% See also SPECTROGRAM

[x,Fs] = audioread('AcGtr.wav');

% Waveform
t = [0:length(x)-1].' * (1/Fs);
subplot(3,1,1);
plot(t,x); axis([0 length(x)*(1/Fs) -1 1]);

% Spectrogram 
nfft = 2048; % length of each time frame

window = hann(nfft); % calculated windowing function

overlap = 128; % Number of samples for frame overlap

% Use the built-in spectrogram function
[y,f,t,p] = spectrogram(x,window,overlap,nfft,Fs);

% Lower Subplot
subplot(3,1,2:3);
surf(t,f,10*log10(p),'EdgeColor','none');
% Rotate the spectrogram to look like a typical "audio" visualization
axis xy; axis tight; view(0,90);
xlabel('Time (sec.)');ylabel('Frequency (Hz)');
````
<!-- END_FILE: Ch_15/audioSpecGram.m -->

<!-- BEGIN_FILE: Ch_15/autoWahExample.m -->
#### `Ch_15/autoWahExample.m`

````matlab
% AUTOWAHEXAMPLE
% This script implements an auto-wah effect using
% a bi-quad filter as the resonant LPF. 
%
% See also BIQUADWAH

clear;clc;
[in,Fs] = audioread('AcGtr.wav');
Ts = 1/Fs;
rate = 0.5; % Hz (frequency of LFO)
centerFreq = 1500; % Hz (center freq of LFO)
depth = 1000; % Hz (LFO range = 500 to 2500)

% Feed-forward Delay Buffer
ff = [0 ; 0]; % ff(n,1) = n-samples of delay  

% Feed-back Delay Buffer
fb = [0 ; 0]; % fb(n,1) = n-samples of delay

% Bandwidth of resonant LPF
Q = 7;

% Wet/Dry Mix
wet = 100;

% Initialize Output Signal
out = zeros(size(in));

for n = 1:length(in)
    t = (n-1) * Ts;
    lfo = depth*sin(2*pi*rate*t) + centerFreq;
    
    % Use Bi-quad Wah Effect Function
    [out(n,1),ff,fb] = biquadWah(in(n,1),Fs,...
        lfo,Q,ff,fb,wet);
    
end

sound(out,Fs);

% Spectrogram - notice resonances in shape of sine wave
nfft = 2048; 
window = hann(nfft); 
overlap = 128;
[y,f,t,p] = spectrogram(out,window,overlap,nfft,Fs);
surf(t,f,10*log10(p),'EdgeColor','none');
axis xy; axis tight; view(0,90);
xlabel('Time (sec.)');ylabel('Frequency (Hz)');
````
<!-- END_FILE: Ch_15/autoWahExample.m -->

<!-- BEGIN_FILE: Ch_15/barberpole2Example.m -->
#### `Ch_15/barberpole2Example.m`

````matlab
% BARBERPOLE2EXAMPLE
% This script creates a barberpole flanger effect using 
% two delay buffers, which gradually fades back and forth
% between the buffers to have a smooth transition at
% the start of the sawtooth ramp.
%
% See also BARBERPOLEEXAMPLE
clear;clc;
Fs = 48000; Ts = 1/Fs;
sec = 8;
lenSamples = sec*Fs;
in = 0.2*randn(lenSamples,1);
t = [0:lenSamples-1].' * (1/Fs);

% Create delay buffer to be hold maximum possible delay time
maxDelay = 50+1; 
buffer = zeros(maxDelay,1); 

rate = 0.5; % Hz (frequency of LFO)
depth = 6; % Samples (amplitude of LFO)
predelay = 12; % Samples (offset of LFO)

% Wet/Dry Mix
wet = 50;  % 0 = Only Dry, 100 = Only Wet


% Initialize Output Signal
N = length(in);
out = zeros(N,1);
lfo1 = zeros(N,1);
lfo2 = zeros(N,1);
overlap = 18000; % Number of samples of overlap per crossfade     
[g1,g2] = crossfades(Fs,lenSamples,rate/2,overlap);

for n = 1:N
    
    % Use flangerEffect.m function
    [out(n,1),buffer,lfo1(n,1),lfo2(n,1)] = ...
        barberpoleFlanger2(in(n,1),buffer,Fs,n,...
        depth,rate,predelay,wet,g1(n,1),g2(n,1));
    
end

sound(out,Fs);

% Waveform
subplot(4,1,1);
plot(t,lfo1,t,lfo2);
axis([0 length(t)*(1/Fs) 8 16]);
ylabel('Delay');

subplot(4,1,2);
plot(t,g1,t,g2); % Cross-fade gains
ylabel('Amplitude');

% Spectrogram 
nfft = 2048; % length of each time frame
window = hann(nfft); % calculated windowing function
overlap = 128; % Number of samples for frame overlap
[y,f,tS,p] = spectrogram(out,window,overlap,nfft,Fs);

% Lower Subplot
subplot(4,1,3:4);
surf(tS,f,10*log10(p),'EdgeColor','none');
axis xy; axis tight; view(0,90);
xlabel('Time (sec.)');ylabel('Frequency (Hz)');
````
<!-- END_FILE: Ch_15/barberpole2Example.m -->

<!-- BEGIN_FILE: Ch_15/barberpoleExample.m -->
#### `Ch_15/barberpoleExample.m`

````matlab
% BARBERPOLEEXAMPLE
%
% This script creates a barberpole flanger effect using 
% a single delay buffer. The delay time is modulated
% by a sawtooth LFO. There is audible distortion each
% time the LFO starts a new cycle. 
%
% This script produces a plot with the delay time 
% of the LFO and the spectrogram of white noise processed
% by the effect.
%
% See also BARBERPOLE2EXAMPLE

clear;clc;
Fs = 48000; Ts = 1/Fs;
sec = 8;
lenSamples = sec*Fs;
in = 0.2*randn(lenSamples,1);
t = [0:lenSamples-1].' * (1/Fs);

% Create delay buffer to be hold maximum possible delay time
maxDelay = 50+1; 
buffer = zeros(maxDelay,1); 

rate = 0.5; % Hz (frequency of LFO)
depth = 6; % Samples (amplitude of LFO)
predelay = 12; % Samples (offset of LFO)

% Wet/Dry Mix
wet = 50;  % 0 = Only Dry, 100 = Only Wet


% Initialize Output Signal
N = length(in);
out = zeros(N,1);
lfo = zeros(N,1);

for n = 1:N
    
    % Use flangerEffect.m function
    [out(n,1),buffer,lfo(n,1)] = ...
        barberpoleFlanger(in(n,1),buffer,Fs,n,...
        depth,rate,predelay,wet);
    
end

sound(out,Fs);

% Waveform
subplot(3,1,1);
plot(t,lfo);
axis([0 length(t)*(1/Fs) 5 20]);
ylabel('Delay');

% Spectrogram 
nfft = 2048; % length of each time frame
window = hann(nfft); % calculated windowing function
overlap = 128; % Number of samples for frame overlap
[y,f,tS,p] = spectrogram(out,window,overlap,nfft,Fs);

% Lower Subplot
subplot(3,1,2:3);
surf(tS,f,10*log10(p),'EdgeColor','none');
axis xy; axis tight; view(0,90);
xlabel('Time (sec.)');ylabel('Frequency (Hz)');
````
<!-- END_FILE: Ch_15/barberpoleExample.m -->

<!-- BEGIN_FILE: Ch_15/barberpoleFlanger.m -->
#### `Ch_15/barberpoleFlanger.m`

````matlab
% BARBERPOLEFLANGER
% This function can be used to create a barber-pole flanger
%
% Input Variables
%   in : single sample of the input signal
%   buffer : used to store delayed samples of the signal
%   n : current sample number used for the LFO
%   depth : range of modulation (samples)
%   rate : speed of modulation (frequency, Hz)
%   predelay : offset of modulation (samples)
%   wet : percent of processed signal (dry = 100 - wet)
%
% See also FLANGEREFFECT, FEEDBACKFLANGER, BARBERPOLEFLANGER2

function [out,buffer,lfo] = barberpoleFlanger(in,buffer,Fs,n,...
    depth,rate,predelay,wet)

% Calculate time in seconds for the current sample
t = (n-1)/Fs;
lfo = depth * sawtooth(2*pi*rate*t,0) + predelay;

% Wet/Dry Mix
mixPercent = wet;  % 0 = Only Dry, 100 = Only Feed-back
mix = mixPercent/100;

fracDelay = lfo; 
intDelay = floor(fracDelay); 
frac = fracDelay - intDelay; 
    
% Store Dry and Wet Signals
drySig = in; 
wetSig = (1-frac)*buffer(intDelay,1) + (frac)*buffer(intDelay+1,1);

% Blend Parallel Paths
out = (1-mix)*drySig + mix*wetSig;

% Feed-back is created by storing the output in the 
% buffer instead of the input

buffer = [in ; buffer(1:end-1,1)]; 
````
<!-- END_FILE: Ch_15/barberpoleFlanger.m -->

<!-- BEGIN_FILE: Ch_15/barberpoleFlanger2.m -->
#### `Ch_15/barberpoleFlanger2.m`

````matlab
% BARBERPOLEFLANGER2
% This function can be used to create a barber-pole flanger.
% Specifically, the function is meant to crossfade between two
% flangers so that the rising flanger has a smooth transition
% at the start of each sawtooth ramp.
%
% Input Variables
%   in : single sample of the input signal
%   buffer : used to store delayed samples of the signal
%   n : current sample number used for the LFO
%   depth : range of modulation (samples)
%   rate : speed of modulation (frequency, Hz)
%   predelay : offset of modulation (samples)
%   wet : percent of processed signal (dry = 100 - wet)
%   g1 : amplitude of the first flanger in the crossfade
%   g2 : amplitude of the second flanger in the crossfade
%
% See also BARBERPOLEFLANGER

function [out,buffer,lfo1,lfo2] = barberpoleFlanger2(in,buffer,...
    Fs,n,depth,rate,predelay,wet,g1,g2)

% Calculate time in seconds for the current sample
t = (n-1)/Fs;
% Rate/2 because alternating, overlapping LFOs
lfo1 = depth * sawtooth(2*pi*rate/2*t + pi/6,0); 
% Hard-clipping at a negative value creates overlap
if lfo1 < -1
    lfo1 = -1;
end
lfo1 = lfo1 + predelay - 2;
lfo2 = depth * sawtooth(2*pi*rate/2*t + 7*pi/6,0); 
if lfo2 < -1
    lfo2 = -1;
end

lfo2 = lfo2 + predelay - 2;

% Wet/Dry Mix
mixPercent = wet;  % 0 = Only Dry, 100 = Only Feed-back
mix = mixPercent/100;

fracDelay1 = lfo1; 
intDelay1 = floor(fracDelay1); 
frac1 = fracDelay1 - intDelay1; 

fracDelay2 = lfo2; 
intDelay2 = floor(fracDelay2); 
frac2 = fracDelay2 - intDelay2;

% Store Dry and Wet Signals
drySig = in; 
wetSig = g1 * ((1-frac1)*buffer(intDelay1,1) + ...
    (frac1)*buffer(intDelay1+1,1)) ...
    + g2 * ((1-frac2)*buffer(intDelay2,1) + ...
    (frac2)*buffer(intDelay2+1,1));

% Blend Parallel Paths
out = (1-mix)*drySig + mix*wetSig;

% Feed-back is created by storing the output in the 
% buffer instead of the input

buffer = [in ; buffer(1:end-1,1)]; 
````
<!-- END_FILE: Ch_15/barberpoleFlanger2.m -->

<!-- BEGIN_FILE: Ch_15/basicPitch.m -->
#### `Ch_15/basicPitch.m`

````matlab
% BASICPITCH
% This script demonstrates a basic example of
% pitch shifting created by 
% using a modulated time delay.
clc;clear;
% Synthesize 1 Hz Test Signal
Fs = 48000; Ts = 1/Fs;
t = [0:Ts:1].'; 
f = 110;             % Musical Note A2 = 110 Hz
in = sin(2*pi*f*t);

% Pitch Shift Amount
semitones = -12;     % (-12,-11,...,-1,0,1,2,...,11,12,...)
tr = 2^(semitones/12);
dRate = 1 - tr;       % Delay Rate of Change

% Conditional to handle pitch up or pitch down
if dRate > 0   % Pitch Decrease
    d = 0;
    x = [in ; zeros(Fs,1)]; % Prepare for signal to be elongated
    
else           % Pitch Increase
    % Initialize delay so it is always positive
    d = length(in)*-dRate;     
    x = in;
end
    
N = length(x);
y = zeros(N,1);
buffer = zeros(Fs*2,1);
for n = 1:N-1
    intDelay = floor(d); 
    frac = d - intDelay;
    if intDelay == 0 
        y(n,1) = (1-frac) * x(n,1) + ...
            frac * buffer(1,1);
    else
        y(n,1) = (1-frac) * buffer(intDelay,1) + ...
            frac * buffer(intDelay+1,1);
    end
    % Store the current output in appropriate index
    buffer = [x(n,1);buffer(1:end-1,1)];
    d = d+dRate;
end
plottf(in,Fs);figure; % A2 = 110 Hz
plottf(y,Fs);         % A1 = 55 Hz
````
<!-- END_FILE: Ch_15/basicPitch.m -->

<!-- BEGIN_FILE: Ch_15/basicPitchDown.m -->
#### `Ch_15/basicPitchDown.m`

````matlab
% BASICPITCHDOWN
% This script demonstrates a basic example of
% pitch shifting down an octave created by 
% using a modulated time delay.
%
% See also BASICPITCHUP, BASICPITCH
clc;clear;
% Synthesize 1 Hz Test Signal
Fs = 48000; Ts = 1/Fs;
t = [0:Ts:1].'; f = 1; 
in = sin(2*pi*f*t);
x = [in ; zeros(Fs,1)];  % Zero-pad input because output 2x length

% Initialize Loop for Pitch Decrease
d = 0;      % Initially start with no delay
N = length(x);
y = zeros(N,1);
buffer = zeros(Fs*2,1);

for n = 1:N   
    intDelay = floor(d); 
    frac = d - intDelay;
    if intDelay == 0   % When there are 0 samples of delay
                       % "y" is based on input "x" 
        y(n,1) = (1-frac) * x(n,1) + ...
            frac * buffer(1,1);
        
    else  % Greater than 0 samples of delay
          % Interpolate between delayed samples "in the past"
        y(n,1) = (1-frac) * buffer(intDelay,1) + ...
            frac * buffer(intDelay+1,1);
    end
    
    % Store the current input in delay buffer
    buffer = [x(n,1);buffer(1:end-1,1)];
    
    % Increase the delay time by 0.5 samples
    d = d + 0.5;
end
plot(t,in); hold on;
time = [0:length(y)-1]*Ts; time = time(:);
plot(time,y); hold off;
xlabel('Time (sec.)');ylabel('Amplitude');
legend('Input','Output');
````
<!-- END_FILE: Ch_15/basicPitchDown.m -->

<!-- BEGIN_FILE: Ch_15/basicPitchUp.m -->
#### `Ch_15/basicPitchUp.m`

````matlab
% BASICPITCHUP
% This script demonstrates a basic example of
% pitch shifting up an octave created by 
% using a modulated time delay.
%
% See also BASICPITCHDOWN, BASICPITCH
clc;clear;
% Synthesize 1 Hz Test Signal
Fs = 48000; Ts = 1/Fs;
t = [0:Ts:1].'; f = 1;
x = sin(2*pi*f*t);

d = Fs;                   % Initial Delay Time      
 
N = length(x);            % Number of samples
y = zeros(N,1);           % Initialize output signal  
buffer = zeros(Fs+1,1);   % Delay Buffer
for n = 1:N
    intDelay = floor(d); 
    frac = d - intDelay;
    if intDelay == 0 
        y(n,1) = (1-frac) * x(n,1) + ...
            frac * buffer(1,1);
    else
        y(n,1) = (1-frac) * buffer(intDelay,1) + ...
            frac * buffer(intDelay+1,1);
    end
    % Store the current input in delay buffer
    buffer = [x(n,1);buffer(1:end-1,1)];
    
    % Decrease the delay time by 1 sample
    d = d - 1;
end
plot(t,x); hold on;
plot(t,y); hold off;
xlabel('Time (sec.)');ylabel('Amplitude');
legend('Input','Output');
````
<!-- END_FILE: Ch_15/basicPitchUp.m -->

<!-- BEGIN_FILE: Ch_15/biquadPhaser.m -->
#### `Ch_15/biquadPhaser.m`

````matlab
% BIQUADPHASER
% This function can be used to create a Phaser audio effect
% by using a bi-quad APF
%
% Input Variables
%   in : single sample of the input signal
%   Fs : sampling rate
%   lfo : used to determine the frequency of APF
%   ff : buffer for feed-foward delay
%   fb : buffer for feed-back delay
%   wet : percent of processed signal (dry = 100 - wet)
%
% Use Table 13.1 to Caculate APF Bi-quad Coefficients
%
% See also PHASEREFFECT, BIQUADWAH

function [out,ff,fb] = biquadPhaser(in,Fs,...
        lfo,Q,ff,fb,wet)

% Convert value of LFO to normalized frequency
w0 = 2*pi*lfo/Fs;
% Normalize Bandwidth
alpha = sin(w0)/(2*Q);

b0 = 1-alpha;    a0 = 1+alpha;
b1 = -2*cos(w0); a1 = -2*cos(w0);
b2 = 1+alpha;    a2 = 1-alpha;

% Wet/Dry Mix
mixPercent = wet;  % 0 = Only Dry, 100 = Only Wet
mix = mixPercent/100;
    
% Store Dry and Wet Signals
drySig = in; 

% All-pass Filter
wetSig = (b0/a0)*in + (b1/a0)*ff(1,1) + ...
    (b2/a0)*ff(2,1) - (a1/a0)*fb(1,1) - (a2/a0)*fb(2,1);

% Blend Parallel Paths
out = (1-mix)*drySig + mix*wetSig;

% Iterate Buffers for Next Sample
ff(2,1) = ff(1,1);
ff(1,1) = in;
fb(2,1) = fb(1,1);
fb(1,1) = wetSig;
````
<!-- END_FILE: Ch_15/biquadPhaser.m -->

<!-- BEGIN_FILE: Ch_15/biquadWah.m -->
#### `Ch_15/biquadWah.m`

````matlab
% BIQUADWAH
% This function can be used to create a Wah-wah audio effect
%
% Input Variables
%   in : single sample of the input signal
%   Fs : sampling rate
%   lfo : used to determine the frequency of LPF
%   ff : buffer for feed-foward delay
%   fb : buffer for feed-back delay
%   wet : percent of processed signal (dry = 100 - wet)
%
% Use Table 13.1 to Caculate LPF Bi-quad Coefficients
%
% See all BIQUADPHASER

function [out,ff,fb] = biquadWah(in,Fs,...
        lfo,Q,ff,fb,wet)



% Convert value of LFO to normalized frequency
w0 = 2*pi*lfo/Fs;
% Normalize Bandwidth
alpha = sin(w0)/(2*Q);

b0 = (1-cos(w0))/2;    a0 = 1+alpha;
b1 = 1-cos(w0);        a1 = -2*cos(w0);
b2 = (1-cos(w0))/2;    a2 = 1-alpha;

% Wet/Dry Mix
mixPercent = wet;  % 0 = Only Dry, 100 = Only Wet
mix = mixPercent/100;
    
% Store Dry and Wet Signals
drySig = in; 

% Low-pass Filter
wetSig = (b0/a0)*in + (b1/a0)*ff(1,1) + ...
    (b2/a0)*ff(2,1) - (a1/a0)*fb(1,1) - (a2/a0)*fb(2,1);

% Blend Parallel Paths
out = (1-mix)*drySig + mix*wetSig;

% Iterate Buffers for Next Sample
ff(2,1) = ff(1,1);
ff(1,1) = in;
fb(2,1) = fb(1,1);
fb(1,1) = wetSig;
````
<!-- END_FILE: Ch_15/biquadWah.m -->

<!-- BEGIN_FILE: Ch_15/chorusEffect.m -->
#### `Ch_15/chorusEffect.m`

````matlab
% CHORUSEFFECT
% This function can be used to create a Chorus audio effect
%
% Input Variables
%   in : single sample of the input signal
%   buffer : used to store delayed samples of the signal
%   n : current sample number used for the LFO
%   depth : range of modulation (milliseconds)
%   rate : speed of modulation (frequency, Hz)
%   predelay : offset of modulation (milliseconds)
%   wet : percent of processed signal (dry = 100 - wet)
%
% See also VIBRATOEFFECT, FLANGEREFFECT

function [out,buffer] = chorusEffect(in,buffer,Fs,n,...
    depth,rate,predelay,wet)

% Calculate time in seconds for the current sample
t = (n-1)/Fs;
lfoMS = depth * sin(2*pi*rate*t) + predelay;
lfoSamples = (lfoMS/1000)*Fs;

% Wet/Dry Mix
mixPercent = wet;  % 0 = Only Dry, 100 = Only Wet
mix = mixPercent/100;

fracDelay = lfoSamples; 
intDelay = floor(fracDelay); 
frac = fracDelay - intDelay; 
    
% Store Dry and Wet Signals
drySig = in; 
wetSig = (1-frac)*buffer(intDelay,1) + (frac)*buffer(intDelay+1,1);

% Blend Parallel Paths
out = (1-mix)*drySig + mix*wetSig;

% Linear Buffer Implemented
buffer = [in ; buffer(1:end-1,1)]; 
````
<!-- END_FILE: Ch_15/chorusEffect.m -->

<!-- BEGIN_FILE: Ch_15/chorusExample.m -->
#### `Ch_15/chorusExample.m`

````matlab
% CHORUSEXAMPLE
% This script creates a chorus effect, applied to an acoustic
% guitar recording. 
%
% See also CHORUSEFFECT
clear;clc;

[in,Fs] = audioread('AcGtr.wav');

maxDelay = ceil(.05*Fs);  % maximum delay of 50 ms
buffer = zeros(maxDelay,1); 

rate = 0.6; % Hz (frequency of LFO)
depth = 5; % Milliseconds (amplitude of LFO)
predelay = 30; % Milliseconds (offset of LFO)

wet = 50; % Percent Wet (Dry = 100 - Wet)

% Initialize Output Signal
N = length(in);
out = zeros(N,1);

for n = 1:N
    
    % Use chorusEffect.m function
    [out(n,1),buffer] = chorusEffect(in(n,1),buffer,Fs,n,...
        depth,rate,predelay,wet);
    
end

sound(out,Fs);
````
<!-- END_FILE: Ch_15/chorusExample.m -->

<!-- BEGIN_FILE: Ch_15/crossfades.m -->
#### `Ch_15/crossfades.m`

````matlab
% CROSSFADES
% This function is used for modulated delay effects which
% require a crossfade between two different delays. The function
% returns the amplitude values for two different paths assuming
% a sawtooth LFO. 
%
% Input variables
%   Fs : sampling rate
%   len : total length in samples for a1 and a2
%   Hz : number of fades per second
%   fade : duration of overlap in samples

function [g1,g2] = crossfades(Fs,len,Hz,fade)

period = Fs/Hz;

win = hann(fade*2);  

n = 1;
g1 = zeros(len,1);
g2 = zeros(len,1);
while n < len
    
    % Position of "n" relative to a cycle
    t = mod(n,period);
    if t < period/2 - fade %fade/2
        g1(n,1) = 1;
        g2(n,1) = 0;
        c = 1;
    elseif t < period/2 %+ fade/2 % 1st fade
        g1(n,1) = win(fade+c,1)^0.5;
        g2(n,1) = win(c,1)^0.5;
        c = c+1;
    elseif t < period - fade
        g1(n,1) = 0;
        g2(n,1) = 1; 
        c = 1;
    else                        % 2nd fade
        g1(n,1) = win(c,1)^0.5;
        g2(n,1) = win(fade+c,1)^(0.5);
        c = c+1;
    end
    n = n + 1;
end
    
    
````
<!-- END_FILE: Ch_15/crossfades.m -->

<!-- BEGIN_FILE: Ch_15/feedbackFlanger.m -->
#### `Ch_15/feedbackFlanger.m`

````matlab
% FEEDBACKFLANGER
% This function can be used to create a feed-back flanger
% 
% Input Variables
%   in : single sample of the input signal
%   buffer : used to store delayed samples of the signal
%   n : current sample number used for the LFO
%   depth : range of modulation (samples)
%   rate : speed of modulation (frequency, Hz)
%   predelay : offset of modulation (samples)
%   wet : percent of processed signal (dry = 100 - wet)
% 
% See also FLANGEREFFECT

function [out,buffer] = feedbackFlanger(in,buffer,Fs,n,...
    depth,rate,predelay,wet)

% Calculate time in seconds for the current sample
t = (n-1)/Fs;
lfo = depth * sin(2*pi*rate*t) + predelay;

% Wet/Dry Mix
mixPercent = wet;  % 0 = Only Dry, 100 = Only Feedback
mix = mixPercent/100;

fracDelay = lfo; 
intDelay = floor(fracDelay); 
frac = fracDelay - intDelay; 
    
% Store Dry and Wet Signals
drySig = in; 
wetSig = (1-frac)*buffer(intDelay,1) + (frac)*buffer(intDelay+1,1);

% Blend Parallel Paths
out = (1-mix)*drySig + mix*wetSig;

% Feedback is created by storing the output in the 
% buffer instead of the input
buffer = [out ; buffer(1:end-1,1)]; 
````
<!-- END_FILE: Ch_15/feedbackFlanger.m -->

<!-- BEGIN_FILE: Ch_15/flangerEffect.m -->
#### `Ch_15/flangerEffect.m`

````matlab
% FLANGEREFFECT
% This function can be used to create a Flanger audio effect
%
% Input Variables
%   in : single sample of the input signal
%   buffer : used to store delayed samples of the signal
%   n : current sample number used for the LFO
%   depth : range of modulation (samples)
%   rate : speed of modulation (frequency, Hz)
%   predelay : offset of modulation (samples)
%   wet : percent of processed signal (dry = 100 - wet)
%
% See also CHORUSEFFECT, FEEDBACKFLANGER, BARBERPOLEFLANGER

function [out,buffer] = flangerEffect(in,buffer,Fs,n,...
    depth,rate,predelay,wet)

% Calculate time in seconds for the current sample
t = (n-1)/Fs;
lfo = depth * sin(2*pi*rate*t) + predelay;

% Wet/Dry Mix
mixPercent = wet;  % 0 = Only Dry, 100 = Only Wet
mix = mixPercent/100;

fracDelay = lfo; 
intDelay = floor(fracDelay); 
frac = fracDelay - intDelay; 
    
% Store Dry and Wet Signals
drySig = in; 
wetSig = (1-frac)*buffer(intDelay,1) + (frac)*buffer(intDelay+1,1);

% Blend Parallel Paths
out = (1-mix)*drySig + mix*wetSig;

% Update Buffer
buffer = [in ; buffer(1:end-1,1)]; 
````
<!-- END_FILE: Ch_15/flangerEffect.m -->

<!-- BEGIN_FILE: Ch_15/flangerExample.m -->
#### `Ch_15/flangerExample.m`

````matlab
% FLANGEREXAMPLE
% This script creates a flanger effect, applied to white noise.
% Within the processing loop, a feed-back flanger can be 
% substituted for the feed-foward flanger used by default.
%
% See also FLANGEREFFECT
clear;clc;
Fs = 48000; Ts = 1/Fs;
sec = 5;
lenSamples = sec*Fs;
in = 0.2*randn(lenSamples,1); % White noise input

% Create delay buffer to be hold maximum possible delay time
maxDelay = 50+1; 
buffer = zeros(maxDelay,1); 

rate = 0.2; % Hz (frequency of LFO)
depth = 4; % Samples (amplitude of LFO)
predelay = 5; % Samples (offset of LFO)
wet = 50; % Wet/Dry Mix

% Initialize Output Signal
N = length(in);
out = zeros(N,1);

for n = 1:N
    
    % Use flangerEffect function
    [out(n,1),buffer] = flangerEffect(in(n,1),buffer,Fs,n,...
        depth,rate,predelay,wet);
    
    % Use feedbackFlanger function
    %[out(n,1),buffer] = feedbackFlanger(in(n,1),buffer,Fs,n,...
    %    depth,rate,predelay,wet);
    
end
sound(out,Fs);

% Spectrogram 
nfft = 2048; 
window = hann(nfft); 
overlap = 128;
[y,f,t,p] = spectrogram(out,window,overlap,nfft,Fs);
surf(t,f,10*log10(p),'EdgeColor','none');
axis xy; axis tight; view(0,90);
xlabel('Time (sec.)');ylabel('Frequency (Hz)');
````
<!-- END_FILE: Ch_15/flangerExample.m -->

<!-- BEGIN_FILE: Ch_15/fractionalDelay.m -->
#### `Ch_15/fractionalDelay.m`

````matlab
function [out,buffer] = fractionalDelay(x,buffer,delay)
    intDelay = floor(delay); 
    frac = delay - intDelay;
    if intDelay == 0 
        out = (1-frac) * x + ...
            frac * buffer(1,1);
    else
        out = (1-frac) * buffer(intDelay,1) + ...
            frac * buffer(intDelay+1,1);
    end
    % Store the current output in appropriate index
    buffer = [x;buffer(1:end-1,1)];
````
<!-- END_FILE: Ch_15/fractionalDelay.m -->

<!-- BEGIN_FILE: Ch_15/harmonyExample.m -->
#### `Ch_15/harmonyExample.m`

````matlab
% HARMONYEXAMPLE
% This script creates a harmony effect by 
% blending together a pitch shifted signal
% with the original, unprocessed signal. 
%
% See also PITCHSHIFTER, PITCHSHIFTEREXAMPLE

clc;clear;
% Import acoustic guitar recording for processing
[in,Fs] = audioread('AcGtr.wav');

% Pitch Shifted Down a Perfect 4th
semitones = -5; 
processed = pitchShifter(in,Fs,semitones);

% Blend Together Input and Processed
out = 0.5 * (in + processed);
sound(out,Fs);
% sound(in,Fs); % For comparison
````
<!-- END_FILE: Ch_15/harmonyExample.m -->

<!-- BEGIN_FILE: Ch_15/lfoPitch.m -->
#### `Ch_15/lfoPitch.m`

````matlab
% LFOPITCH
% This script demonstrates an example of pitch shifting
% using a sawtooth LFO to modulate delay time. 
% The result is a signal which has been pitch shifted
% based on the "semitones" variable. 
%
% An important aspect of this algorithm is to
% avoid having the processed signal be a different
% length than the original signal. To make this
% possible, the maximum delay time is 50 ms. A
% sawtooth LFO is used to modulate the delay time
% between 0 ms and 50 ms based on the necessary
% rate of change. If the delay time is about to 
% go outside of this range, a new cycle of the 
% sawtooth begins. 
%
% The output signal has audible clicks and pops 
% due to the discontinuities of the modulated delay.
% This motivates the use of two parallel delay lines
% which crossfade back and forth to smooth over the
% discontinuities.
%
% See also BASICPITCH, PITCHSHIFTER, PITCHSHIFTEREXAMPLE

clc;clear;close all;
% Synthesize 1 Hz Test Signal
[in,Fs] = audioread('AcGtr.wav');
Ts = 1/Fs;

semitones = 1;     % (-12,-11,...,-1,0,1,2,...,11,12,...)
tr = 2^(semitones/12);
dRate = 1 - tr;       % Delay Rate of Change

maxDelay = Fs * .05;  % Maximum Delay is 50 ms

% Conditional to handle pitch up and pitch down
if dRate > 0   % Pitch Decrease
    d = 0;
    
else           % Pitch Increase
    % Initialize delay so it is always positive
    d = maxDelay;     
end
    
N = length(in);
out = zeros(N,1);
lfo = zeros(N,1);
buffer = zeros(maxDelay+1,1);
for n = 1:N
    % Determine output of delay buffer
    % which could be a fractional delay time
    intDelay = floor(d); 
    frac = d - intDelay;
    
    if intDelay == 0 % When delay time = zero, 
                     % "out" comes "in", not just delay buffer
        out(n,1) = (1-frac) * in(n,1) + ...
            frac * buffer(1,1);
    else
        out(n,1) = (1-frac) * buffer(intDelay,1) + ...
            frac * buffer(intDelay+1,1);
    end
    
    % Store the current output in appropriate index
    buffer = [in(n,1);buffer(1:end-1,1)];
    
    % Store the current delay in signal for plotting 
    lfo(n,1) = d;
    d = d+dRate; % Change the delay time for the next loop
    
    % If necessary, start a new cycle in LFO
    if d < 0     
        d = maxDelay;
    elseif d > maxDelay
        d = 0;
    end
end
sound(out,Fs);

t = [0:N-1]*Ts;
subplot(3,1,1);
plot(t,lfo); % Cross-fade gains
ylabel('Delay (Samples)'); axis tight;

% Spectrogram 
nfft = 2048; % length of each time frame
window = hann(nfft); % calculated windowing function
overlap = 128; % Number of samples for frame overlap
[y,f,tS,p] = spectrogram(out,window,overlap,nfft,Fs);
subplot(3,1,2:3);
surf(tS,f,10*log10(p),'EdgeColor','none');
axis xy; axis tight; view(0,90);
xlabel('Time (sec.)');ylabel('Frequency (Hz)');

% This figure plots the equivalent sawtooth signal
% to the LFO synthesized during the loop
figure;
tau = (maxDelay/dRate) * Ts;
f = 1/tau;
plot(t,lfo,t,maxDelay*(0.5*sawtooth(2*pi*f*t)+0.5),'r--');
axis tight;
````
<!-- END_FILE: Ch_15/lfoPitch.m -->

<!-- BEGIN_FILE: Ch_15/phaserEffect.m -->
#### `Ch_15/phaserEffect.m`

````matlab
% PHASEREFFECT
% This function can be used to create a Phaser audio effect
%
% Input Variables
%   in : single sample of the input signal
%   buffer : used to store delayed samples of the signal
%   n : current sample number used for the LFO
%   depth : range of modulation (samples)
%   rate : speed of modulation (frequency, Hz)
%   wet : percent of processed signal (dry = 100 - wet)
%
% See also BIQUADPHASER

function [out,buffer] = phaserEffect(in,buffer,Fs,n,...
    depth,rate,wet)

% Calculate time in seconds for the current sample
t = (n-1)/Fs;
lfo = depth * sin(2*pi*rate*t)+2;

% Wet/Dry Mix
mixPercent = wet;  % 0 = Only Dry, 100 = Only Wet
mix = mixPercent/100;

fracDelay = lfo; 
intDelay = floor(fracDelay); 
frac = fracDelay - intDelay; 
    
% Store Dry and Wet Signals
drySig = in; 

g = 0.25;
% All-pass Filter
wetSig = g*in + ((1-frac)*buffer(intDelay,1) + ...
    frac*buffer(intDelay+1,1));

% Blend Parallel Paths
out = (1-mix)*drySig + mix*wetSig;

buffer = [in + -g*wetSig ; buffer(1:end-1,1)];
````
<!-- END_FILE: Ch_15/phaserEffect.m -->

<!-- BEGIN_FILE: Ch_15/phaserExample.m -->
#### `Ch_15/phaserExample.m`

````matlab
% PHASEREXAMPLE
% This script demonstrates the use of a phaser
% function to add the effect to white noise. Parameters
% of the phaser effect include the rate and depth of 
% the LFO. In this implementation, the delay time of
% an APF (Direct Form II) is modulated.
%
% See also PHASEREFFECT

clear;clc;
Fs = 48000; Ts = 1/Fs;
sec = 5;
lenSamples = sec*Fs;
in = 0.2*randn(lenSamples,1);

rate = 0.8; % Hz (frequency of LFO)
depth = 0.3; % Samples (amplitude of LFO)

% Initialize Delay Buffers
buffer = zeros(3,1); % All-pass Filter

% Wet/Dry Mix
wet = 50;

% Initialize Output Signal
out = zeros(size(in));

for n = 1:length(in)
    
    % Use Phaser Effect Function
    [out(n,1),buffer] = phaserEffect(in(n,1),buffer,Fs,n,...
    depth,rate,wet);
    
end

sound(out,Fs);

% Spectrogram 
nfft = 2048; 
window = hann(nfft); 
overlap = 128;
[y,f,t,p] = spectrogram(out,window,overlap,nfft,Fs);
surf(t,f,10*log10(p),'EdgeColor','none');
axis xy; axis tight; view(0,90); %view(-30,60);
xlabel('Time (sec.)');ylabel('Frequency (Hz)');
````
<!-- END_FILE: Ch_15/phaserExample.m -->

<!-- BEGIN_FILE: Ch_15/phaserExample2.m -->
#### `Ch_15/phaserExample2.m`

````matlab
% PHASEREXAMPLE2
% This script implements a phaser effect using
% a bi-quad filter as the APF. 
%
% See also BIQUADPHASER

clear;clc;
Fs = 48000; Ts = 1/Fs;
sec = 5;
lenSamples = sec*Fs;
in = 0.2*randn(lenSamples,1);

rate = 0.8; % Hz (frequency of LFO)
centerFreq = 1000; % Hz (center freq of LFO)
depth = 500; % Hz (LFO range = 500 to 1500)

% Feed-forward Delay Buffer
ff = [0 ; 0]; % ff(n,1) = n-samples of delay  

% Feed-back Delay Buffer
fb = [0 ; 0]; % fb(n,1) = n-samples of delay

% Bandwidth of Phaser (wide or narrow notch)
Q = 0.5;

% Wet/Dry Mix
wet = 50;

% Initialize Output Signal
out = zeros(size(in));

for n = 1:length(in)
    t = (n-1) * Ts;
    lfo = depth*sin(2*pi*rate*t) + centerFreq;
    
    % Use Bi-quad Phaser Effect Function
    [out(n,1),ff,fb] = biquadPhaser(in(n,1),Fs,...
        lfo,Q,ff,fb,wet);
    
end

sound(out,Fs);

% Spectrogram 
nfft = 2048; 
window = hann(nfft); 
overlap = 128;
[y,f,t,p] = spectrogram(out,window,overlap,nfft,Fs);
surf(t,f,10*log10(p),'EdgeColor','none');
axis xy; axis tight; view(0,90); %view(-30,60);
xlabel('Time (sec.)');ylabel('Frequency (Hz)');
````
<!-- END_FILE: Ch_15/phaserExample2.m -->

<!-- BEGIN_FILE: Ch_15/pitchShifter.m -->
#### `Ch_15/pitchShifter.m`

````matlab
% PITCHSHIFTER
% This function implements the pitch shifter
% audio effect by using two parallel delay lines.
% The delay time for each line is modulated by
% a sawtooth LFO. The frequency of the LFO is
% based on the desired number of semitones for 
% the pitch shifter. Both increases and decreases
% in pitch are possible with this function. 
%
% Both LFOs repeat a cycle such that the delay
% time stays within a range of 0 ms to 50 ms.
% This way the processed signal is not significantly
% shorter or longer than the original signal. 
%
% The cycles of the two LFOS are intentionally
% offset to have an overlap. An amplitude crossfade
% is applied to the delay lines to switch between
% the two during the overlap. The crossfade reduces
% the audibility of the relatively large discontinuity
% in the delay time at the start of each LFO cycle.
%
% See also PITCHSHIFTEREXAMPLE, CROSSFADES, LFOPITCH

function [out] = pitchShifter(in,Fs,semitones)
Ts = 1/Fs;
N = length(in);      % Total number of samples
out = zeros(N,1);
lfo1 = zeros(N,1);   % For visualizing the LFOs
lfo2 = zeros(N,1);

maxDelay = Fs * .05;  % Maximum Delay is 50 ms
buffer1 = zeros(maxDelay+1,1);   
buffer2 = zeros(maxDelay+1,1);    % Initialize Delay Buffers

tr = 2^(semitones/12);     % Convert Semitones 
dRate = 1 - tr;            % Delay Rate of Change

tau = (maxDelay/abs(dRate))*Ts;  % Period of Sawtooth LFO
freq = 1/tau;                    % Frequency of LFO

fade = round((tau*Fs)/8);  % Fade length is 1/8th of a cycle
Hz = (freq/2)*(8/7);       % Frequency of crossfade due to overlap
[g1,g2] = crossfades(Fs,N,Hz,fade); % Crossfade Gains

if dRate > 0   % Pitch Decrease
     % Initialize delay so LFO cycles line up with crossfade
    d1 = dRate * fade;  
    d2 = maxDelay; 
    d1Temp = d1;  % These variables are used to control 
    d2Temp = d2;  % the length of each cycle of the LFO
                  % for the proper amount of overlap
                  
else           % Pitch Increase
    % Initialize delay so LFO cycles line up with crossfade
    d1 = maxDelay - maxDelay/8;   
    d2 = 0;
    d1Temp = d1;
    d2Temp = d2;
end

% Loop to Process Input Signal
for n = 1:N
    % Parallel delay processing of the input signal
    [out1,buffer1] = fractionalDelay(in(n,1),buffer1,d1);
    [out2,buffer2] = fractionalDelay(in(n,1),buffer2,d2);
    
    % Use crossfade gains to combine the output of each delay
    out(n,1) = g1(n,1)*out1 + g2(n,1)*out2;
    
    lfo1(n,1) = d1;   % Save the current delay time 
    lfo2(n,1) = d2;   % for plotting
    
    % The following conditions are set up to control the 
    % overlap of the sawtooth LFOs
    if dRate < 0 % Slope of LFO is negative (Pitch up)
        d1 = d1 + dRate;
        d1Temp = d1Temp + dRate;
        if d1 < 0 
            d1 = 0; % Portion of LFO where delay time = 0
        end
        if d1Temp < -maxDelay * (6/8) % Start next cycle
            d1 = maxDelay;
            d1Temp = maxDelay;
        end
            
        d2 = d2 + dRate;   
        d2Temp = d2Temp + dRate;
        if d2 < 0 
            d2 = 0; % Portion of LFO where delay time = 0
        end
        if d2Temp < -maxDelay * (6/8) % Start new cycle
            d2 = maxDelay;
            d2Temp = maxDelay;
        end
        
    else  % Slope of LFO is positive (Pitch Down)
       
        d1Temp = d1Temp + dRate;
        if d1Temp > maxDelay % Start next cycle
            d1 = 0;
            d1Temp = -maxDelay * (6/8);
        elseif d1Temp < 0
            d1 = 0;      % Portion where delay time = 0
        else
            d1 = d1 + dRate;
        end
            
        
        d2Temp = d2Temp + dRate;
        if d2Temp > maxDelay 
            d2 = 0;
            d2Temp = -maxDelay * (6/8);
        elseif d2Temp < 0
            d2 = 0;
        else
            d2 = d2 + dRate;
        end
        
    end
end


% Uncomment for plotting
% t = [0:N-1]*Ts;
% subplot(4,1,1);  % Waveform
% plot(t,lfo1,t,lfo2);
% axis([0 t(end) -100 maxDelay]); ylabel('Delay');
% 
% subplot(4,1,2);
% plot(t,g1,t,g2); % Cross-fade gains
% axis([0 t(end) -0.1 1.1]);ylabel('Amplitude');
% 
% 
% nfft = 2048; % length of each time frame
% window = hann(nfft); % calculated windowing function
% overlap = 128; % Number of samples for frame overlap
% [y,f,tS,p] = spectrogram(out,window,overlap,nfft,Fs);
% 
% 
% subplot(4,1,3:4); % Lower Subplot Spectrogram 
% surf(tS,f,10*log10(p),'EdgeColor','none');
% axis xy; axis tight; view(0,90);
% xlabel('Time (sec.)');ylabel('Frequency (Hz)');
````
<!-- END_FILE: Ch_15/pitchShifter.m -->

<!-- BEGIN_FILE: Ch_15/pitchShifterExample.m -->
#### `Ch_15/pitchShifterExample.m`

````matlab
% PITCHSHIFTEREXAMPLE
% This script is an example to demonstrate
% the pitchShifter function for processing
% an audio signal. The desired number of 
% semitones can be set from within this script.
%
% See also PITCHSHIFTER, LFOPITCH, BASICPITCH 

clc;clear;
% Import acoustic guitar recording for processing
[in,Fs] = audioread('AcGtr.wav');

% Experiment with different values
% -12,-11,...,-1,0,1,2,...,11,12,...
semitones = 1;     

[out] = pitchShifter(in,Fs,semitones);

sound(out,Fs);
% sound(in,Fs); % For comparison
````
<!-- END_FILE: Ch_15/pitchShifterExample.m -->

<!-- BEGIN_FILE: Ch_15/plottf.m -->
#### `Ch_15/plottf.m`

````matlab
%PLOTTF Plot sampled signal in time and frequency domains
%   PLOTTF(x,Fs) plots the time-domain samples in vector x, assuming that 
%   Fs is an audio sampling rate (44.1k, 48k, etc.) in samples/second, 
%   and also plots the the Fourier transform on the decibel scale
%   between the frequencies of 20 Hz and 20 kHz, logarithmically spaced.
%
% See also PLOT

function plottf(x,Fs)
Ts = 1/Fs;
N = length(x);
t = [0:N-1]*Ts; t=t(:);

subplot(2,1,1);
plot(t,x); xlabel('Time (sec.)'); ylabel('Amplitude');

% Fourier Transform
len = N;
if len < 4096
    len = 4096;
end
X=(2/N)*fft(x,len);        % do DFT/FFT

f= [0:len-1]*(Fs/len);

% Ensure there will be no values of -Inf dB
% by making the minimum value = -120 dB
X(abs(X)<0.000001) = 0.000001;

subplot(2,1,2);
semilogx(f,20*log10(abs(X))); axis([20 20000 -60 4]);
ax = gca; ax.XTick =[20 50 100 200 300 500 1000 2000 5000 10000 20000];
xlabel('Frequency (Hz)'); ylabel('Amplitude (dB)');
````
<!-- END_FILE: Ch_15/plottf.m -->

<!-- BEGIN_FILE: Ch_15/vibratoEffect.m -->
#### `Ch_15/vibratoEffect.m`

````matlab
% VIBRATOEFFECT
% This function implements a vibrato effect based
% on depth and rate LFO parameters.
%
% Input Variables
%   in : single sample of the input signal
%   buffer : used to store delayed samples of the signal
%   n : current sample number used for the LFO
%   depth : range of modulation (samples)
%   rate : speed of modulation (frequency, Hz)
%
% See also CHORUSEFFECT

function [out,buffer] = vibratoEffect(in,buffer,Fs,n,...
    depth,rate)

% Calculate lfo for current sample
t = (n-1)/Fs;
lfo = (depth/2) * sin(2*pi*rate*t) + depth; 

% Determine indexes for circular buffer
len = length(buffer);
indexC = mod(n-1,len) + 1; % Current index in circular buffer

fracDelay = mod(n-lfo-1,len) + 1; % Delay index in circular buffer
intDelay = floor(fracDelay);      % Fractional delay indices
frac = fracDelay - intDelay;

nextSamp = mod(intDelay,len) + 1;  % Next index in circular buffer


out = (1-frac) * buffer(intDelay,1) + ...
    (frac) * buffer(nextSamp,1);

% Store the current output in appropriate index
buffer(indexC,1) = in;
````
<!-- END_FILE: Ch_15/vibratoEffect.m -->

<!-- BEGIN_FILE: Ch_15/vibratoExample.m -->
#### `Ch_15/vibratoExample.m`

````matlab
% VIBRATOEXAMPLE
% This script creates a vibrato effect, applied to an acoustic
% guitar recording. Parameters for the effect include "rate"
% and "depth" which can be used to control the intensity
% of the vibrato. At the end of the script, the sound of
% the result is played.
%
% See also VIBRATOEFFECT
clear;clc;

[in,Fs] = audioread('AcGtr.wav'); % Input signal
Ts = 1/Fs;
N = length(in);

% Initialize the delay buffer
maxDelay = 1000; % Samples
buffer = zeros(maxDelay,1); 

% LFO parameters
t = [0:N-1]*Ts; t = t(:);
rate = 4; % Frequency of LFO in Hz
depth = 75; % Range of samples of delay

% Initialize Output Signal
out = zeros(N,1);

for n = 1:N
    
   [out(n,1),buffer] = vibratoEffect(in(n,1),buffer,Fs,n,...
    depth,rate);
    
end

sound(out,Fs);
````
<!-- END_FILE: Ch_15/vibratoExample.m -->

<!-- END_CHAPTER: Ch_15 -->

<!-- BEGIN_CHAPTER: Ch_16 -->
## Ch_16

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_16/apf.m` | 54 | 1749 |
| code | `Ch_16/apfExample.m` | 47 | 1204 |
| code | `Ch_16/crossoverFeedback.m` | 70 | 1780 |
| code | `Ch_16/earlyReflections.m` | 58 | 1968 |
| code | `Ch_16/fbcf.m` | 51 | 1725 |
| code | `Ch_16/fbcfExample.m` | 50 | 1380 |
| code | `Ch_16/fbcfNoMod.m` | 43 | 1405 |
| code | `Ch_16/fbcfParallelExample.m` | 58 | 1647 |
| code | `Ch_16/fbcfSeriesExample.m` | 57 | 1561 |
| code | `Ch_16/fdnExample.m` | 90 | 2605 |
| code | `Ch_16/lpcf.m` | 56 | 1851 |
| code | `Ch_16/modDelay.m` | 49 | 1642 |
| code | `Ch_16/moorerReverb.m` | 95 | 2703 |
| code | `Ch_16/rt60.m` | 93 | 2688 |
| code | `Ch_16/schroederReverb.m` | 81 | 2216 |

### Source Files

<!-- BEGIN_FILE: Ch_16/apf.m -->
#### `Ch_16/apf.m`

````matlab
% APF
% This function creates an all-pass filter by 
% processing an individual input sample and updating 
% a delay buffer used in a loop to index each sample
% in a signal.
%
% Input Variables
%   n : current sample number of the input signal
%   delay : samples of delay
%   gain : feed-back gain (linear scale)
%   amp : amplitude of LFO modulation
%   rate : frequency of LFO modulation

function [out,buffer] = apf(in,buffer,Fs,n,delay,gain,amp,rate)

% Calculate time in seconds for the current sample
t = (n-1)/Fs;
fracDelay = amp * sin(2*pi*rate*t);
intDelay = floor(fracDelay); 
frac = fracDelay - intDelay; 

% Determine indexes for circular buffer
len = length(buffer);
indexC = mod(n-1,len) + 1; % Current index 
indexD = mod(n-delay-1+intDelay,len) + 1; % Delay index
indexF = mod(n-delay-1+intDelay+1,len) + 1; % Fractional index

% Temp variable for output of delay buffer
w = (1-frac)*buffer(indexD,1) + (frac)*buffer(indexF,1);

% Temp variable used for the node after the input sum
v = in + (-gain*w);

% Summation at output
out = (gain * v) + w;

% Store the current input to delay buffer
buffer(indexC,1) = v;

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/apf.m -->

<!-- BEGIN_FILE: Ch_16/apfExample.m -->
#### `Ch_16/apfExample.m`

````matlab
% APFEXAMPLE
% This script uses an all-pass filter
% function, applied to an acoustic guitar recording. 
%
% See also APF
clear;clc;

[in,Fs] = audioread('AcGtr.wav');

maxDelay = max(ceil(.05*Fs));  % maximum delay of 50 ms
buffer = zeros(maxDelay,1); 

d = ceil(.042*Fs); % 42 ms of delay
g = 0.9;

rate = 0.9; % Hz (frequency of LFO)
amp = 6; % Range of +,- 6 samples for delay

% Initialize Output Signal
N = length(in);
out = zeros(N,1);

for n = 1:N
    
    % Use apf.m function
    [out(n,1),buffer] = apf(in(n,1),buffer,Fs,n,...
        d,g,amp,rate);
    
end

sound(out,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/apfExample.m -->

<!-- BEGIN_FILE: Ch_16/crossoverFeedback.m -->
#### `Ch_16/crossoverFeedback.m`

````matlab
% CROSSOVERFEEDBACK
% This script implements two comb filters with
% crossover feedback
%
% See also MODDELAY
clear;clc;

Fs = 48000;
in = [1 ; zeros(Fs*0.5,1)];

% Max delay of 70 ms
maxDelay = ceil(.07*Fs);  
% Initialize all buffers
buffer1 = zeros(maxDelay,1); buffer2 = zeros(maxDelay,1);  

% Delay and Gain Parameters
d1 = fix(.0297*Fs); 
d2 = fix(.0419*Fs); 
g11 = -0.75; g12 = -0.75;
g21 = -0.75; g22 = -0.75;

% LFO parameters
rate1 = 0.6; amp1 = 3; 
rate2 = 0.71; amp2 = 3;

% Initialize Output Signal
N = length(in);
out = zeros(N,1);

fb1 = 0; fb2 = 0; % Feed-back holding variables

for n = 1:N
    
    % Combine input with feed-back for respective delay lines 
    inDL1 = in(n,1) + fb1;
    inDL2 = in(n,1) + fb2;
    
    % Two Parallel Delay Lines
    
    [outDL1,buffer1] = modDelay(inDL1,buffer1,Fs,n,...
    d1,amp1,rate1);
    
    [outDL2,buffer2] = modDelay(inDL2,buffer2,Fs,n,...
    d2,amp2,rate2);
    
    % Combine parallel paths
    out(n,1) = 0.5*(outDL1 + outDL2);
    
    % Calculate Feed-back (including crossover)
    fb1 = 0.5*(g11 * outDL1 + g21 * outDL2);
    fb2 = 0.5*(g12 * outDL1 + g22 * outDL2);
    
end
plot(out);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/crossoverFeedback.m -->

<!-- BEGIN_FILE: Ch_16/earlyReflections.m -->
#### `Ch_16/earlyReflections.m`

````matlab
% EARLYREFLECTIONS
% This function creates a tapped delay line to 
% be used for the Early Reflections of a reverb algorithm.
% The delays and gains of the taps are included in this 
% function, and were based on an IR measurement from a 
% recording studio in Nashville, TN. 
%
% See also MOORERREVERB

function [out,buffer] = earlyReflections(in,buffer,Fs,n)

% Delay times converted from milliseconds
delayTimes = fix(Fs*[0; 0.01277; 0.01283; 0.01293; 0.01333;...
    0.01566; 0.02404; 0.02679; 0.02731; 0.02737; 0.02914; ...
    0.02920; 0.02981; 0.03389; 0.04518; 0.04522; ...
    0.04527; 0.05452; 0.06958]);
            
% There must be a "gain" for each of the "delayTimes"
gains = [1; 0.1526; -0.4097; 0.2984; 0.1553; 0.1442;...
    -0.3124; -0.4176; -0.9391; 0.6926; -0.5787; 0.5782; ...
     0.4206; 0.3958; 0.3450; -0.5361; 0.417; 0.1948; 0.1548];          

% Determine indexes for circular buffer
len = length(buffer);
indexC = mod(n-1,len) + 1; % Current index 
buffer(indexC,1) = in;

out = 0; % Initialize the output to be used in loop

% Loop through all the taps
for tap = 1:length(delayTimes)
    % Find the circular buffer index for the current tap
    indexTDL = mod(n-delayTimes(tap,1)-1,len) + 1;  
   
    % "Tap" the delay line and add current tap with output
    out = out + gains(tap,1) * buffer(indexTDL,1);
    
end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/earlyReflections.m -->

<!-- BEGIN_FILE: Ch_16/fbcf.m -->
#### `Ch_16/fbcf.m`

````matlab
% FBCF
% This function creates a feed-back comb filter by 
% processing an individual input sample and updating 
% a delay buffer used in a loop to index each sample
% in a signal. Fractional delay is implemented to make
% it possible to modulate the delay time.
%
% Input Variables
%   n : current sample number of the input signal
%   delay : samples of delay
%   fbGain : feed-back gain (linear scale)
%   amp : amplitude of LFO modulation
%   rate : frequency of LFO modulation
%
% See also FBCFNOMOD

function [out,buffer] = fbcf(in,buffer,Fs,n,delay,fbGain,amp,rate)

% Calculate time in seconds for the current sample
t = (n-1)/Fs;
fracDelay = amp * sin(2*pi*rate*t);
intDelay = floor(fracDelay); 
frac = fracDelay - intDelay; 

% Determine indexes for circular buffer
len = length(buffer);
indexC = mod(n-1,len) + 1; % Current index 
indexD = mod(n-delay-1+intDelay,len) + 1; % Delay index
indexF = mod(n-delay-1+intDelay+1,len) + 1; % Fractional index

out = (1-frac)*buffer(indexD,1) + (frac)*buffer(indexF,1);

% Store the current output in appropriate index
buffer(indexC,1) = in + fbGain*out;

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/fbcf.m -->

<!-- BEGIN_FILE: Ch_16/fbcfExample.m -->
#### `Ch_16/fbcfExample.m`

````matlab
% FBCFEXAMPLE
% This script uses a feed-back comb filter (FBCF)
% function, applied to an acoustic guitar recording. 
%
% See also FBCFNOMOD, FBCF
clear;clc;

[in,Fs] = audioread('AcGtr.wav');

maxDelay = ceil(.05*Fs);  % maximum delay of 50 ms
buffer = zeros(maxDelay,1);    % initialize delay buffer

d = .04*Fs; % 40 ms of delay
g = -0.7;   % Feed-back gain value

rate = 0.6; % Hz (frequency of LFO)
amp = 6; % Range of +,- 6 samples for delay

% Initialize Output Signal
N = length(in);
out = zeros(N,1);

for n = 1:N
    
    % uncomment to use fbcfNoMod.m function
    %[out(n,1),buffer] = fbcfNoMod(in(n,1),buffer,n,d,g);
    
    % Use fbcf.m function
    [out(n,1),buffer] = fbcf(in(n,1),buffer,Fs,n,...
        d,g,amp,rate);
    
end

sound(out,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/fbcfExample.m -->

<!-- BEGIN_FILE: Ch_16/fbcfNoMod.m -->
#### `Ch_16/fbcfNoMod.m`

````matlab
% FBCFNOMOD
% This function creates a feed-back comb filter by 
% processing an individual input sample and updating 
% a delay buffer used in a loop to index each sample
% in a signal. This implementation does not use
% fractional delay. Therefore, the delay time
% cannot be modulated.
%
% Input Variables
%   n : current sample number of the input signal
%   delay : samples of delay
%   fbGain : feed-back gain (linear scale)
%
% See also FBCF

function [out,buffer] = fbcfNoMod(in,buffer,n,delay,fbGain)

% Determine indexes for circular buffer
len = length(buffer);
indexC = mod(n-1,len) + 1; % Current index 
indexD = mod(n-delay-1,len) + 1; % Delay index

out = buffer(indexD,1);

% Store the current output in appropriate index
buffer(indexC,1) = in + fbGain*buffer(indexD,1);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/fbcfNoMod.m -->

<!-- BEGIN_FILE: Ch_16/fbcfParallelExample.m -->
#### `Ch_16/fbcfParallelExample.m`

````matlab
% FBCFPARALLELEXAMPLE
% This script uses parallel feed-back comb filter (FBCF)
% functions, applied to an acoustic guitar recording.
%
% See also FBCFSERIESEXAMPLE
clear;clc;

[in,Fs] = audioread('AcGtr.wav');

maxDelay = ceil(.07*Fs);  % max delay of 70 ms
buffer1 = zeros(maxDelay,1); 
buffer2 = zeros(maxDelay,1); 

d1 = fix(.047*Fs); % 47 ms of delay
g1 = 0.5;
d2 = fix(.053*Fs); % 53 ms of delay
g2 = -0.5;

rate1 = 0.6; % Hz (frequency of LFO)
amp1 = 6; % Range of +- 6 samples for delay
rate2 = 0.5; % Hz (frequency of LFO)
amp2 = 8; % Range of +- 8 samples for delay

% Initialize Output Signal
N = length(in);
out = zeros(N,1);

for n = 1:N
    
    % Two Parallel FBCFs
    [w1,buffer1] = fbcf(in(n,1),buffer1,Fs,n,...
        d1,g1,amp1,rate1);
    % Both FBCFs receive "in" to create
    % parallel processing
    [w2,buffer2] = fbcf(in(n,1),buffer2,Fs,n,...
        d2,g2,amp2,rate2);
    % The output of each FBCF is summed together
    % to complete parallel processing
    out(n,1) = w1 + w2;
end

sound(out,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/fbcfParallelExample.m -->

<!-- BEGIN_FILE: Ch_16/fbcfSeriesExample.m -->
#### `Ch_16/fbcfSeriesExample.m`

````matlab
% FBCFSERIESEXAMPLE
% This script uses series feed-back comb filter (FBCF)
% functions, applied to an acoustic guitar recording. 
%
% See also FBCFPARALLELEXAMPLE
clear;clc;

[in,Fs] = audioread('AcGtr.wav');

maxDelay = ceil(.07*Fs);  % max delay of 70 ms
buffer1 = zeros(maxDelay,1); 
buffer2 = zeros(maxDelay,1); 

d1 = fix(.042*Fs); % 42 ms of delay
g1 = 0.5;
d2 = fix(.053*Fs); % 53 ms of delay
g2 = -0.5;

rate1 = 0.6; % Hz (frequency of LFO)
amp1 = 6; % Range of +- 6 samples for delay
rate2 = 0.5; % Hz (frequency of LFO)
amp2 = 8; % Range of +- 8 samples for delay

% Initialize Output Signal
N = length(in);
out = zeros(N,1);

for n = 1:N
    
    % Two Series FBCFs
    [w,buffer1] = fbcf(in(n,1),buffer1,Fs,n,...
        d1,g1,amp1,rate1);
    
    % The output "w" of the first FBCF is used
    % as the input to the second FBCF
    [out(n,1),buffer2] = fbcf(w,buffer2,Fs,n,...
        d2,g2,amp2,rate2);
    
end

sound(out,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/fbcfSeriesExample.m -->

<!-- BEGIN_FILE: Ch_16/fdnExample.m -->
#### `Ch_16/fdnExample.m`

````matlab
% FDNEXAMPLE
% This script implements a feed-back delay network
% using a Stautner and Puckette matrix.
%
% See also MODDELAY
clear;clc;

[in,Fs] = audioread('AcGtr.wav');
% Add extra space at end for the reverb tail
in = [in;zeros(Fs*3,1)]; 

% Max delay of 70 ms
maxDelay = ceil(.07*Fs);  
% Initialize all buffers
buffer1 = zeros(maxDelay,1); buffer2 = zeros(maxDelay,1); 
buffer3 = zeros(maxDelay,1); buffer4 = zeros(maxDelay,1);  

% Delay and Gain Parameters
d1 = fix(.0297*Fs); 
d2 = fix(.0371*Fs); 
d3 = fix(.0411*Fs); 
d4 = fix(.0437*Fs); 
g11 = 0; g12 = 1; g13 = 1; g14 = 0;  % Stautner and Puckette
g21 =-1; g22 = 0; g23 = 0; g24 =-1;  % Feed-back Matrix
g31 = 1; g32 = 0; g33 = 0; g34 =-1;
g41 = 0; g42 = 1; g43 =-1; g44 = 0;

% LFO parameters
rate1 = 0.6; amp1 = 5; 
rate2 = 0.71; amp2 = 5;
rate3 = 0.83; amp3 = 5; 
rate4 = 0.95; amp4 = 5;

% Initialize Output Signal
N = length(in);
out = zeros(N,1);

% Feed-back holding variables
fb1 = 0; fb2 = 0; fb3 = 0; fb4 = 0;

% Gain to control reverb time
g = .67;
for n = 1:N
    
    % Combine input with feed-back for respective delay lines 
    inDL1 = in(n,1) + fb1;
    inDL2 = in(n,1) + fb2;
    inDL3 = in(n,1) + fb3;
    inDL4 = in(n,1) + fb4;
    
    % Four Parallel Delay Lines
    [outDL1,buffer1] = modDelay(inDL1,buffer1,Fs,n,...
    d1,amp1,rate1);
    
    [outDL2,buffer2] = modDelay(inDL2,buffer2,Fs,n,...
    d2,amp2,rate2);

    [outDL3,buffer3] = modDelay(inDL3,buffer3,Fs,n,...
    d3,amp3,rate3);
    
    [outDL4,buffer4] = modDelay(inDL4,buffer4,Fs,n,...
    d4,amp4,rate4);
    
    % Combine parallel paths
    out(n,1) = 0.25*(outDL1 + outDL2 + outDL3 + outDL4);
    
    % Calculate Feed-back (including cross-over)
    fb1 = g*(g11*outDL1 + g21*outDL2 + g31*outDL3 + g41*outDL4);
    fb2 = g*(g12*outDL1 + g22*outDL2 + g32*outDL3 + g42*outDL4);
    fb3 = g*(g13*outDL1 + g23*outDL2 + g33*outDL3 + g43*outDL4);
    fb4 = g*(g14*outDL1 + g24*outDL2 + g34*outDL3 + g44*outDL4);
    
end
sound(out,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/fdnExample.m -->

<!-- BEGIN_FILE: Ch_16/lpcf.m -->
#### `Ch_16/lpcf.m`

````matlab
% LPCF
% This function creates a feed-back comb filter 
% with a LPF in the feed-back path.
%
% Input Variables
%   n : current sample number of the input signal
%   delay : samples of delay
%   fbGain : feed-back gain (linear scale)
%   amp : amplitude of LFO modulation
%   rate : frequency of LFO modulation
%   fbLPF : output delayed one sample to create basic LPF
%
% See also MOORERREVERB

function [out,buffer,fbLPF] = lpcf(in,buffer,Fs,n,delay,...
    fbGain,amp,rate,fbLPF)

% Calculate time in seconds for the current sample
t = (n-1)/Fs;
fracDelay = amp * sin(2*pi*rate*t);
intDelay = floor(fracDelay); 
frac = fracDelay - intDelay; 

% Determine indexes for circular buffer
len = length(buffer);
indexC = mod(n-1,len) + 1; % Current index 
indexD = mod(n-delay-1+intDelay,len) + 1; % Delay index
indexF = mod(n-delay-1+intDelay+1,len) + 1; % Fractional index

out = (1-frac)*buffer(indexD,1) + (frac)*buffer(indexF,1);

% Store the current output in appropriate index
% The LPF is created by adding the current output
% with the previous sample, both are weighted 0.5
buffer(indexC,1) = in + fbGain*(0.5*out + 0.5*fbLPF);

% Store the current output for the Feed-back LPF 
% to be used with the next sample
fbLPF = out;

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/lpcf.m -->

<!-- BEGIN_FILE: Ch_16/modDelay.m -->
#### `Ch_16/modDelay.m`

````matlab
% MODDELAY
% This function creates a series delay effect 
% using a buffer. The delay time can be modulated 
% based on the LFO parameters "depth" and "rate"
%
% Input Variables
%   in : single sample of the input signal
%   buffer : used to store delayed samples of the signal
%   n : current sample number used for the LFO
%   depth : range of modulation (samples)
%   rate : speed of modulation (frequency, Hz)

function [out,buffer] = modDelay(in,buffer,Fs,n,...
    delay,depth,rate)

% Calculate time in seconds for the current sample
t = (n-1)/Fs;
fracDelay = depth * sin(2*pi*rate*t);
intDelay = floor(fracDelay); 
frac = fracDelay - intDelay; 

% Determine indexes for circular buffer
len = length(buffer);
indexC = mod(n-1,len) + 1; % Current index 
indexD = mod(n-delay-1+intDelay,len) + 1; % Delay index
indexF = mod(n-delay-1+intDelay+1,len) + 1; % Fractional index


out = (1-frac) * buffer(indexD,1) + ...
    (frac) * buffer(indexF,1);

% Store the current output in appropriate index
buffer(indexC,1) = in;

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/modDelay.m -->

<!-- BEGIN_FILE: Ch_16/moorerReverb.m -->
#### `Ch_16/moorerReverb.m`

````matlab
% MOORERREVERB
% This script implements the Moorer Reverb algorithm
% by modifying the Schroeder Reverb script. First,
% an additional step to add Early Reflections is included.
% Second, a simple low-pass filter is included in the feed-back 
% path of the comb filters.
%
% See also EARLYREFLECTIONS, LPCF

clear;clc;

[in,Fs] = audioread('AcGtr.wav');
in = [in;zeros(Fs*3,1)]; % Add zero-padding for reverb tail
    
% Max delay of 70 ms
maxDelay = ceil(.07*Fs);  
% Initialize all buffers
buffer1 = zeros(maxDelay,1); buffer2 = zeros(maxDelay,1); 
buffer3 = zeros(maxDelay,1); buffer4 = zeros(maxDelay,1); 
buffer5 = zeros(maxDelay,1); buffer6 = zeros(maxDelay,1); 

% Early Reflections Tapped Delay Line
bufferER = zeros(maxDelay,1);

% Delay and Gain Parameters
d1 = fix(.0297*Fs); g1 = 0.9;
d2 = fix(.0371*Fs); g2 = -0.9;
d3 = fix(.0411*Fs); g3 = 0.9;
d4 = fix(.0437*Fs); g4 = -0.9;
d5 = fix(.005*Fs); g5 = 0.7;
d6 = fix(.0017*Fs); g6 = 0.7;

% LFO parameters
rate1 = 0.6; amp1 = 8; 
rate2 = 0.71; amp2 = 8;
rate3 = 0.83; amp3 = 8; 
rate4 = 0.95; amp4 = 8;
rate5 = 1.07; amp5 = 8; 
rate6 = 1.19; amp6 = 8;

% Variables used as delay for a simple LPF in each Comb Filter function
fbLPF1 = 0; fbLPF2 = 0; fbLPF3 = 0; fbLPF4 = 0;

% Initialize Output Signal
N = length(in);
out = zeros(N,1);

for n = 1:N
    
    % Early Reflections TDL
    [w0 , bufferER] = earlyReflections(in(n,1),bufferER,Fs,n);
    
    % Four Parallel LPCFs
    [w1,buffer1,fbLPF1] = lpcf(w0,buffer1,Fs,n,...
        d1,g1,amp1,rate1,fbLPF1);
    
    [w2,buffer2,fbLPF2] = lpcf(w0,buffer2,Fs,n,...
        d2,g2,amp2,rate2,fbLPF2);
    
    [w3,buffer3,fbLPF3] = lpcf(w0,buffer3,Fs,n,...
        d3,g3,amp3,rate3,fbLPF3);
    
    [w4,buffer4,fbLPF4] = lpcf(w0,buffer4,Fs,n,...
        d4,g4,amp4,rate4,fbLPF4);
    
    % Combine parallel paths
    combPar = 0.25*(w1 + w2 + w3 + w4);
    
    % Two Series All-pass Filters
    [w5,buffer5] = apf(combPar,buffer5,Fs,n,...
        d5,g5,amp5,rate5);
    
    [out(n,1),buffer6] = apf(w5,buffer6,Fs,n,...
        d6,g6,amp6,rate6);
    
end

sound(out,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/moorerReverb.m -->

<!-- BEGIN_FILE: Ch_16/rt60.m -->
#### `Ch_16/rt60.m`

````matlab
% RT60
% This script analyzes the RT-60
% of a feed-back delay network by visualizing
% an impulse response on the decibel scale.
clear;clc;

Fs = 48000; Ts = 1/Fs;
in = [ 1 ; zeros(5*Fs,1)]; % Impulse Signal

% Max delay of 70 ms
maxDelay = ceil(.07*Fs);  
% Initialize all buffers
buffer1 = zeros(maxDelay,1); buffer2 = zeros(maxDelay,1); 
buffer3 = zeros(maxDelay,1); buffer4 = zeros(maxDelay,1);  

% Delay and Gain Parameters
d1 = fix(.0297*Fs); 
d2 = fix(.0371*Fs); 
d3 = fix(.0411*Fs); 
d4 = fix(.0437*Fs); 
g11 = 0; g12 = 1; g13 = 1; g14 = 0;
g21 =-1; g22 = 0; g23 = 0; g24 =-1;
g31 = 1; g32 = 0; g33 = 0; g34 =-1;
g41 = 0; g42 = 1; g43 =-1; g44 = 0;

% LFO parameters
rate1 = 0.6; amp1 = 5; 
rate2 = 0.71; amp2 = 5;
rate3 = 0.83; amp3 = 5; 
rate4 = 0.95; amp4 = 5;

% Initialize Output Signal
N = length(in);
out = zeros(N,1);

% Feed-back holding variables
fb1 = 0; fb2 = 0; fb3 = 0; fb4 = 0;

% Gain to control reverb time
g = .67;
for n = 1:N
    
    % Combine input with feed-back for respective delay lines 
    inDL1 = in(n,1) + fb1;
    inDL2 = in(n,1) + fb2;
    inDL3 = in(n,1) + fb3;
    inDL4 = in(n,1) + fb4;
    
    % Four Parallel Delay Lines
    [outDL1,buffer1] = modDelay(inDL1,buffer1,Fs,n,...
    d1,amp1,rate1);
    
    [outDL2,buffer2] = modDelay(inDL2,buffer2,Fs,n,...
    d2,amp2,rate2);

    [outDL3,buffer3] = modDelay(inDL3,buffer3,Fs,n,...
    d3,amp3,rate3);
    
    [outDL4,buffer4] = modDelay(inDL4,buffer4,Fs,n,...
    d4,amp4,rate4);
    
    % Combine parallel paths
    out(n,1) = 0.25*(outDL1 + outDL2 + outDL3 + outDL4);
    
    % Calculate Feed-back (including cross-over)
    fb1 = g*(g11*outDL1 + g21*outDL2 + g31*outDL3 + g41*outDL4);
    fb2 = g*(g12*outDL1 + g22*outDL2 + g32*outDL3 + g42*outDL4);
    fb3 = g*(g13*outDL1 + g23*outDL2 + g33*outDL3 + g43*outDL4);
    fb4 = g*(g14*outDL1 + g24*outDL2 + g34*outDL3 + g44*outDL4);
    
end
out = out/max(abs(out)); % Normalize to Unity Gain (0 dB)

t = [0:N-1]*Ts;
plot(t,20*log10(abs(out))); 
line([0 4],[-60 -60],'Color','red','LineStyle','--');
axis([0 4 -80 0]);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/rt60.m -->

<!-- BEGIN_FILE: Ch_16/schroederReverb.m -->
#### `Ch_16/schroederReverb.m`

````matlab
% SCHROEDERREVERB
% This script implements the Schroeder Reverb algorithm
% by using feed-back comb filters (fbcf) and all-passs
% filters (apf)
%
% See also FBCF, APF
clear;clc;

[in,Fs] = audioread('AcGtr.wav');

% Max delay of 70 ms
maxDelay = ceil(.07*Fs);  
% Initialize all buffers (there are 6 total = 4 FBCF, 2 APF)
buffer1 = zeros(maxDelay,1); buffer2 = zeros(maxDelay,1); 
buffer3 = zeros(maxDelay,1); buffer4 = zeros(maxDelay,1); 
buffer5 = zeros(maxDelay,1); buffer6 = zeros(maxDelay,1); 

% Delay and Gain Parameters
d1 = fix(.0297*Fs); g1 = 0.75;
d2 = fix(.0371*Fs); g2 = -0.75;
d3 = fix(.0411*Fs); g3 = 0.75;
d4 = fix(.0437*Fs); g4 = -0.75;
d5 = fix(.005*Fs); g5 = 0.7;
d6 = fix(.0017*Fs); g6 = 0.7;

% LFO parameters
rate1 = 0.6; amp1 = 8; 
rate2 = 0.71; amp2 = 8;
rate3 = 0.83; amp3 = 8; 
rate4 = 0.95; amp4 = 8;
rate5 = 1.07; amp5 = 8; 
rate6 = 1.19; amp6 = 8;

% Initialize Output Signal
N = length(in);
out = zeros(N,1);

for n = 1:N
    
    % Four Parallel FBCFs
    [w1,buffer1] = fbcf(in(n,1),buffer1,Fs,n,...
        d1,g1,amp1,rate1);
    
    [w2,buffer2] = fbcf(in(n,1),buffer2,Fs,n,...
        d2,g2,amp2,rate2);
    
    [w3,buffer3] = fbcf(in(n,1),buffer3,Fs,n,...
        d3,g3,amp3,rate3);
    
    [w4,buffer4] = fbcf(in(n,1),buffer4,Fs,n,...
        d4,g4,amp4,rate4);
    
    % Combine parallel paths
    combPar = 0.25*(w1 + w2 + w3 + w4);
    
    % Two Series All-pass Filters
    [w5,buffer5] = apf(combPar,buffer5,Fs,n,...
        d5,g5,amp5,rate5);
    
    [out(n,1),buffer6] = apf(w5,buffer6,Fs,n,...
        d6,g6,amp6,rate6);
    
end

sound(out,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_16/schroederReverb.m -->

<!-- END_CHAPTER: Ch_16 -->

<!-- BEGIN_CHAPTER: Ch_17 -->
## Ch_17

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_17/adsr.m` | 53 | 1737 |
| code | `Ch_17/adsrExample.m` | 40 | 1052 |
| code | `Ch_17/biquadWah.m` | 63 | 1753 |
| code | `Ch_17/ciSimulation.m` | 83 | 1995 |
| code | `Ch_17/envelopeModulation.m` | 43 | 1259 |
| code | `Ch_17/envWahExample.m` | 77 | 1943 |
| code | `Ch_17/transientAnalysis.m` | 81 | 2362 |
| code | `Ch_17/transientDesigner.m` | 81 | 2546 |
| code | `Ch_17/transientExample.m` | 35 | 1009 |
| code | `Ch_17/vocoderExample.m` | 92 | 2478 |
| asset | `Ch_17/funkyGtr.wav` | - | 220064 |
| asset | `Ch_17/Synth.wav` | - | 523682 |
| asset | `Ch_17/Voice.wav` | - | 523682 |

### Source Files

<!-- BEGIN_FILE: Ch_17/adsr.m -->
#### `Ch_17/adsr.m`

````matlab
% ADSR
% This function can be used to apply an
% adsr envelope on to an input signal.
%
% Input Variables
%   attackTime : length of attack ramp in milliseconds
%   decayTime : length of decay ramp in ms
%   sustainAmplitude : linear amplitude of sustain segment
%   releaseTime : length of release ramp in ms

function [ y ] = adsr( x,Fs,attackTime,decayTime,...
    sustainAmplitude,releaseTime) 

%Convert time inputs to seconds 
attackTimeS = attackTime / 1000;
decayTimeS = decayTime / 1000;
releaseTimeS = releaseTime / 1000;

%Convert seconds to samples and determine sustain time
a = round(attackTimeS * Fs);     % Round each to an integer
d = round(decayTimeS * Fs);      % number of samples
r = round(releaseTimeS * Fs);
s = length(x) - (a+d+r);       % Determine length of sustain

%Create linearly spaced fades for A,D, and R. Creates hold for S
aFade = linspace(0,1,a)';
dFade = linspace(1,sustainAmplitude,d)';
sFade = sustainAmplitude * ones(s,1);
rFade = linspace(sustainAmplitude,0,r)';

%Concactenates total ADSR curve
adsr = [aFade;dFade;sFade;rFade];

%Applies ADSR shaping to X
y = x .* adsr;

end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_17/adsr.m -->

<!-- BEGIN_FILE: Ch_17/adsrExample.m -->
#### `Ch_17/adsrExample.m`

````matlab
% ADSREXAMPLE
% This script creates a linear ADSR amplitude envelope
%
% See also ADSR

% Number of Samples per fade
a = 20;
d = 20;
s = 70;
r = 40;

sustainAmplitude = 0.75;

% Create each segment A,D,S,R
aFade = linspace(0,1,a)';
dFade = linspace(1,sustainAmplitude,d)';
sFade = sustainAmplitude * ones(s,1);
rFade = linspace(sustainAmplitude,0,r)';


%Concactenates total ADSR envelope
env = [aFade;dFade;sFade;rFade];

plot(env);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_17/adsrExample.m -->

<!-- BEGIN_FILE: Ch_17/biquadWah.m -->
#### `Ch_17/biquadWah.m`

````matlab
% BIQUADWAH
% This function can be used to create a Wah-wah audio effect
%
% Input Variables
%   in : single sample of the input signal
%   Fs : sampling rate
%   env : used to determine the frequency of LPF
%   ff : buffer for feed-foward delay
%   fb : buffer for feed-back delay
%   wet : percent of processed signal (dry = 100 - wet)
%
% Use Table 13.1 to Caculate LPF Bi-quad Coefficients
%
% See also ENVWAHEXAMPLE

function [out,ff,fb] = biquadWah(in,Fs,...
        env,Q,ff,fb,wet)

% Convert value of ENV to normalized frequency
w0 = 2*pi*env/Fs;
% Normalize Bandwidth
alpha = sin(w0)/(2*Q);

b0 = (1-cos(w0))/2;    a0 = 1+alpha;
b1 = 1-cos(w0);        a1 = -2*cos(w0);
b2 = (1-cos(w0))/2;    a2 = 1-alpha;

% Wet/Dry Mix
mixPercent = wet;  % 0 = Only Dry, 100 = Only Wet
mix = mixPercent/100;
    
% Store Dry and Wet Signals
drySig = in; 

% Low-pass Filter
wetSig = (b0/a0)*in + (b1/a0)*ff(1,1) + ...
    (b2/a0)*ff(2,1) - (a1/a0)*fb(1,1) - (a2/a0)*fb(2,1);

% Blend Parallel Paths
out = (1-mix)*drySig + mix*wetSig;

% Iterate Buffers for Next Sample
ff(2,1) = ff(1,1);
ff(1,1) = in;
fb(2,1) = fb(1,1);
fb(1,1) = wetSig;

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_17/biquadWah.m -->

<!-- BEGIN_FILE: Ch_17/ciSimulation.m -->
#### `Ch_17/ciSimulation.m`

````matlab
% CISIMULATION
% This script performs vocoding
% using speech and white noise. This process
% is used to simulate cochlear implants
% for listeners with acoustic hearing
%
% See also VOCODEREXAMPLE

clc;clear;close all;

% Import Audio Files
[in,Fs] = audioread('Voice.wav');
N = length(in);
noise = 0.1 * randn(N,1);

% Initialize filter parameters
Nyq = Fs/2;
order = 2; % Filter Order

numOfBands = 16;

% Logarithmically space cut-off frequencies
% 2*10^1 - 2*10^4 (20-20k) Hz
freq = 2 * logspace(1,4,numOfBands+1);

g = 0.9992; % Smoothing Filter Gain
fb = 0;     % Initialize feedback delay 

voxBands = zeros(N,numOfBands);
noiseBands = zeros(N,numOfBands);
envBands = zeros(N,numOfBands);

for band = 1:numOfBands
   
    Wn = [freq(band) , freq(band+1)] ./ Nyq;
    [b,a] = butter(order,Wn);
    
    % Filterbank
    voxBands(:,band) = filter(b,a,in);
    noiseBands(:,band) = filter(b,a,noise);
    
    % Envelope Measurement
    for n = 1:N
    
        envBands(n,band) = (1-g) * abs(voxBands(n,band)) + g * fb;
        fb = envBands(n,band);
    
    end
    fb = 0;
    
end

% Perform Amplitude Modulation
outBands = zeros(length(in),numOfBands);
for band = 1:numOfBands
   
    outBands(:,band) = envBands(:,band) .* noiseBands(:,band); 
    
end

% Sum together all the bands
out = sum(outBands,2);
% Make-up Gain
out = 32 * out;

sound(out,Fs); plot(out);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_17/ciSimulation.m -->

<!-- BEGIN_FILE: Ch_17/envelopeModulation.m -->
#### `Ch_17/envelopeModulation.m`

````matlab
% ENVELOPEMODULATION
% This script demonstrates the process of measuring an
% amplitude envelope from the waveform of a voice recording
% and using it modulate the amplitude of synth recording.

% Import Audio Files
[in,Fs] = audioread('Voice.wav');
[synth] = audioread('Synth.wav');

alpha = 0.9997;  % Feed-back gain
fb = 0;          % Initialized value for feedback
N = length(in);
env = zeros(N,1);
for n = 1:N
    % Analyze envelope
    env(n,1) = (1-alpha) * abs(in(n,1)) + alpha * fb;
    fb = env(n,1);
    
end
% Make-up Gain
env = 4*env;

% Amplitude Modulation of envelope applied to synthsizer
out = synth.*env;

sound(out,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_17/envelopeModulation.m -->

<!-- BEGIN_FILE: Ch_17/envWahExample.m -->
#### `Ch_17/envWahExample.m`

````matlab
% ENVWAHEXAMPLE
% This script implements an env-wah effect using
% a bi-quad filter as the resonant LPF after 
% analyzing the amplitude envelope of the input
% signal. 
%
% See also BIQUADWAH

clear;clc;
[x,Fs] = audioread('AcGtr.wav');
Ts = 1/Fs;
N = length(x);
% Initialize Feed-forward Delay Buffer 
% (stores 2 previous samples of input)
ff = [0 ; 0]; % ff(n,1) = n-samples of delay  

% Initialize Feed-back Delay Buffer 
% (stores 2 previous samples of output)
fb = [0 ; 0]; % fb(n,1) = n-samples of delay

% Bandwidth of resonant LPF
Q = 4;

% Wet/Dry Mix
wet = 100;

% Initialize Output Signal
y = zeros(N,1);
% Cut-off frequency from envelope
cutoff = zeros(N,1);  

% Envelope LPF Parameters
alpha = 0.9995;
envPreviousValue = 0;

for n = 1:N
    % Envelope Detection
    rect = abs(x(n,1));
    env = (1-alpha) * rect + (alpha)*envPreviousValue;
    envPreviousValue = env;
    
    % Scale Envelope for Cut-off Frequency of LPF
    freq = 1500 + 10000 * env;
    
    % Use Bi-quad Wah Effect Function
    [y(n,1),ff,fb] = biquadWah(x(n,1),Fs,...
        freq,Q,ff,fb,wet);
    % Store for Plotting
    cutoff(n,1) = freq;
end

sound(y,Fs);

t= [0:N-1]*Ts;

subplot(2,1,1);
plot(t,y);
xlabel('Time (sec.)');ylabel('Amplitude');
subplot(2,1,2);
plot(t,cutoff);
xlabel('Time (sec.)');ylabel('Cut-off Freq. (Hz)');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_17/envWahExample.m -->

<!-- BEGIN_FILE: Ch_17/transientAnalysis.m -->
#### `Ch_17/transientAnalysis.m`

````matlab
% TRANSIENTANALYSIS
% This script plots the amplitude envelopes used
% in the transient designer effect. A comparison
% is plotted of the "fast" and "slow" envelopes
% used to determine when "attack" and "sustain"
% is occurring in the signal.
%
% See also TRANSIENTDESIGNER, TRANSIENTEXAMPLE
clear; clc;

[in,Fs]=audioread('AcGtr.wav');

gFast = 0.9991; % Gain smoothing for the "fast" envelope
fbFast = 0;     % Feed-back for the "fast" envelope
gSlow = 0.9999; % Gain smoothing for the "slow" envelope 
fbSlow = 0;     % Feed-back for the "slow" envelope    
N = length(in);
envFast = zeros(N,1);
envSlow = zeros(N,1);
transientShaper = zeros(N,1);
for n = 1:N
    
    envFast(n,1) = (1-gFast) * 2 * abs(in(n,1)) + gFast * fbFast;
    fbFast = envFast(n,1);
    
    envSlow(n,1) = (1-gSlow) * 3 * abs(in(n,1)) + gSlow * fbSlow;
    fbSlow = envSlow(n,1);
    
    transientShaper(n,1) = envFast(n,1) - envSlow(n,1);
    
end

figure(1);
plot(envFast); hold on;
plot(envSlow); 
plot(transientShaper);
hold off;
legend({'$\alpha$ = 0.9991','$\alpha$ = 0.9999', ...
    'envFast - envSlow'},'Interpreter','latex','FontSize',14);
axis([1 length(in) -0.5 1]);

attack = zeros(N,1);
sustain = zeros(N,1);
for n = 1:N
   
    if transientShaper(n,1) > 0
        
        attack(n,1) = transientShaper(n,1) + 1;
        sustain(n,1) = 1;
    else
        
        attack(n,1) = 1;
        sustain(n,1) = transientShaper(n,1) + 1;
        
    end
    
end

figure(2);
subplot(2,1,1);  % Plot the detected attack envelope
plot(attack); title('Attack Envelope', 'FontSize',14);
axis([1 length(in) 0.5 1.5]);
subplot(2,1,2);  % Plot the detected sustain envelope
plot(sustain); title('Sustain Envelope', 'FontSize',14);
axis([1 length(in) 0.5 1.5]);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_17/transientAnalysis.m -->

<!-- BEGIN_FILE: Ch_17/transientDesigner.m -->
#### `Ch_17/transientDesigner.m`

````matlab
% TRANSIENTDESIGNER
% This function implements the transient designer
% audio effect. First, a detection analysis is performed
% to determine the sections of the signal which should 
% be labeled "attack" and "sustain". Then the amplitude
% of these sections is scaled based on the input parameters
%
% Input Variables
%   attack : amount to change transient (-1 dec, 0 unity, +1 inc)
%   sustain : amount to change sustain
%
% See also TRANSIENTANALYSIS


function [out] = transientDesigner(in,attack,sustain)

N = length(in);
% Initialize Filtering Parameters
gFast = 0.9991;   % Feed-back gain for the "fast" envelope
fbFast = 0;       % Variable used to store previous envelope value
gSlow = 0.9999;   % Feed-back gain for "slow" envelope
fbSlow = 0;
envFast = zeros(N,1);
envSlow = zeros(N,1);
differenceEnv = zeros(N,1);

% Measure Fast and Slow Envelopes
for n = 1:N
    
    envFast(n,1) = (1-gFast) * 2 * abs(in(n,1)) + gFast * fbFast;
    fbFast = envFast(n,1);
    
    envSlow(n,1) = (1-gSlow) * 3 * abs(in(n,1)) + gSlow * fbSlow;
    fbSlow = envSlow(n,1);
    
    % Create the difference envelope between "fast" and "slow"
    differenceEnv(n,1) = envFast(n,1) - envSlow(n,1);
    % Note: difference envelope will have a positive value
    % when envFast is greater than envSlow. This occurs
    % when the signal is in "attack". If the difference
    % envelope is negative, then the signal is in
    % "sustain".
end

attEnv = zeros(N,1);
susEnv = zeros(N,1);
% Separate Attack and Sustain Envelopes
for n = 1:N
   
    if differenceEnv(n,1) > 0 % "Attack" section
        
        attEnv(n,1) = (attack * differenceEnv(n,1)) + 1;
        susEnv(n,1) = 1; % No change
        
    else % "Sustain" section
        
        attEnv(n,1) = 1; % No change
        susEnv(n,1) = (sustain * -differenceEnv(n,1)) + 1;
        
    end
    
end

% Apply the Attack and Sustain Envelopes
out = (in .* attEnv) .* susEnv;

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_17/transientDesigner.m -->

<!-- BEGIN_FILE: Ch_17/transientExample.m -->
#### `Ch_17/transientExample.m`

````matlab
% TRANSIENTEXAMPLE
% This script demonstrates the Transient Designer function
%
% See also TRANSIENTDESIGNER, TRANSIENTANALYSIS
clear; clc;

[in,Fs]=audioread('AcGtr.wav');

% Attack and Sustain Parameters [-1,+1]
attack = 0;
sustain = 1;

out = transientDesigner(in,attack,sustain);

plot(out,'r'); hold on;
plot(in,'b'); hold off;
legend({'Output','Input'},'FontSize',14);

sound(out,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_17/transientExample.m -->

<!-- BEGIN_FILE: Ch_17/vocoderExample.m -->
#### `Ch_17/vocoderExample.m`

````matlab
% VOCODEREXAMPLE
% This script demonstrates the process of creating
% a vocoder effect using a Voice signal and
% Synth signal

clc;clear;close all;


% Import Audio Files
[in,Fs] = audioread('Voice.wav');
[synth] = audioread('Synth.wav');

% Initialize filter parameters
Nyq = Fs/2;   % Nyquist frequency
order = 2;    % Filter order

numOfBands = 16;

% Logarithmically space cut-off frequencies
% 2*10^1 - 2*10^4 (20-20k) Hz
freq = 2 * logspace(1,4,numOfBands+1);

g = 0.9992; % Smoothing Filter Gain
fb = 0;     % Initialize feedback delay 

N = length(in);

% These arrrays are used to store the filtered
% versions of the input signal. Each column stores
% the signal for each band. As an example, 
% voxBands(:,4) stores the band-pass filtered
% signal in 4th band. 
voxBands = zeros(N,numOfBands);
synthBands = zeros(N,numOfBands);
envBands = zeros(N,numOfBands);

for band = 1:numOfBands % Perform processing 1 band per loop
   
    % Determine lower and upper cut-off frequencies
    % of the current BPF on a normalized scale.
    Wn = [freq(band) , freq(band+1)] ./ Nyq;
    [b,a] = butter(order,Wn);
    
    % Filter signals and store the result 
    voxBands(:,band) = filter(b,a,in);
    synthBands(:,band) = filter(b,a,synth);
    
    % Envelope measurement from vocal signal
    for n = 1:N
    
        envBands(n,band) = (1-g) * abs(voxBands(n,band)) + g * fb;
        fb = envBands(n,band);
    
    end
    fb = 0;
    
end

% Perform Amplitude Modulation
outBands = zeros(length(in),numOfBands);
for band = 1:numOfBands
    
    % Apply the envelope of the vocal signal to the synthesizer
    % in each of the bands
    outBands(:,band) = envBands(:,band) .* synthBands(:,band); 
    
end

% Sum together all the bands
out = sum(outBands,2);
% Make-up Gain
out = 32 * out;

% Listen to the output and plot it
sound(out,Fs); plot(out);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_17/vocoderExample.m -->

<!-- END_CHAPTER: Ch_17 -->

<!-- BEGIN_CHAPTER: Ch_18 -->
## Ch_18

### Manifest

| Type | Path | Lines | Bytes |
|---|---|---:|---:|
| code | `Ch_18/basicComp.m` | 88 | 2798 |
| code | `Ch_18/biquadFilter.m` | 192 | 4989 |
| code | `Ch_18/biquadStep.m` | 51 | 1433 |
| code | `Ch_18/compressor.m` | 94 | 2821 |
| code | `Ch_18/compressorExample.m` | 40 | 1146 |
| code | `Ch_18/expander.m` | 94 | 2712 |
| code | `Ch_18/expanderExample.m` | 36 | 1127 |
| code | `Ch_18/feedbackComp.m` | 100 | 2937 |
| code | `Ch_18/rmsComp.m` | 97 | 2603 |
| code | `Ch_18/rmsComp2.m` | 95 | 2660 |
| code | `Ch_18/sidechainComp.m` | 94 | 2811 |
| code | `Ch_18/stepDemo.m` | 40 | 1340 |
| code | `Ch_18/stepDesign.m` | 72 | 1994 |
| code | `Ch_18/stepDesignExample.m` | 47 | 1421 |
| code | `Ch_18/stepResponse.m` | 43 | 1375 |
| asset | `Ch_18/Kick.wav` | - | 523682 |
| asset | `Ch_18/monoDrums.wav` | - | 1560044 |
| asset | `Ch_18/Synth.wav` | - | 523682 |

### Source Files

<!-- BEGIN_FILE: Ch_18/basicComp.m -->
#### `Ch_18/basicComp.m`

````matlab
% BASICCOMP
% This script creates a dynamic range compressor with
% attack and release times linked together. A step
% input signal is synthesized for testing. A plot
% is produced at the end of the script to show
% a comparison of the input step signal, the output
% response, and the gain reduction curve.
%
% See also COMPRESSOR, COMPRESSOREXAMPLE

% Step Input Signal
Fs = 48000; Ts = 1/Fs;
x = [zeros(Fs,1); ones(Fs,1); zeros(Fs,1)]; 
N = length(x);
% Parameters for Compressor
T = -12;   % Threshold = -12 dBFS
R = 3;     % Ratio = 3:1
responseTime = 0.25;  % time in seconds
alpha = exp(-log(9)/(Fs * responseTime));
gainSmoothPrev = 0; % Initialize smoothing variable

y = zeros(N,1);
lin_A = zeros(N,1);
% Loop over each sample to see if it is above thresh
for n = 1:N
    %%%%%% Calculations of the Detection Path
    % Turn the input signal into a uni-polar signal on the dB scale
    x_uni = abs(x(n,1));
    x_dB = 20*log10(x_uni/1);
    % Ensure there are no values of negative infinity
    if x_dB < -96
        x_dB = -96;
    end
    
    % Static Characteristics
    if x_dB > T
        gainSC = T + (x_dB - T)/R; % Perform Downwards Compression
    else 
        gainSC = x_dB; % Do not perform compression 
    end
          
    gainChange_dB = gainSC - x_dB;
    
    % smooth over the gainChange_dB to alter Response Time
    gainSmooth =  ((1-alpha)*gainChange_dB) + (alpha*gainSmoothPrev);

    % Convert to linear amplitude scalar
    lin_A(n,1) = 10^(gainSmooth/20);
    
    %%%%%% Apply linear amplitude from Detection Path
    %%%%%% to input sample
    y(n,1) = lin_A(n,1) * x(n,1);
    
    % Update gainSmoothPrev used in the next sample of the loop
    gainSmoothPrev = gainSmooth;
end
t = [0:N-1]*Ts; t = t(:);

subplot(3,1,1);
plot(t,x); title('Step Input');axis([0 3 -0.1 1.1]); 
subplot(3,1,2);
plot(t,y); title('Comp Out'); axis([0 3 -0.1 1.1]);
subplot(3,1,3);
plot(t,lin_A); title('Gain Reduction');axis([0 3 -0.1 1.1]);
% The "gain reduction" line shows the amount of compression
% applied at each sample of the signal. When the value
% is "1", there is no compression. When the value is less
% than "1", gain reduction is happening.

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/basicComp.m -->

<!-- BEGIN_FILE: Ch_18/biquadFilter.m -->
#### `Ch_18/biquadFilter.m`

````matlab
% BIQUADFILTER
% This function implements a bi-quad filter based
% on the Audio EQ Cookbook Coefficients. All filter
% types can be specified (LPF, HPF, BPF, etc.) and
% three different topologies are included.
%
% Input Variables
%   f0 : filter frequency (cut-off or center based on filter)
%   Q : bandwidth parameter 
%   dBGain : gain value on the decibel scale
%   type : 'lpf','hpf','pkf','bp1','bp2','apf','lsf','hsf'
%   form : 1 (Direct Form I), 2 (DFII), 3 (Transposed DFII)

function [out] = biquadFilter(in,Fs,f0,Q,dBGain,type,form)

%%% Initial Parameters
N = length(in);
out = zeros(length(in),1);

%%% Intermediate Variables
%
w0 = 2*pi*f0/Fs;            % Angular Freq. (Radians/sample) 
alpha = sin(w0)/(2*Q);      % Filter Width
A  = sqrt(10^(dBGain/20));  % Amplitude

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% TYPE - LPF,HPF,BPF,APF,HSF,LSF,PKF,NCH
%
%----------------------
%        LPF
%----------------------
if strcmp(type,'lpf')
    b0 =  (1 - cos(w0))/2;
    b1 =   1 - cos(w0);
    b2 =  (1 - cos(w0))/2;
    a0 =   1 + alpha;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha;

%----------------------
%        HPF
%----------------------
elseif strcmp(type,'hpf')
    b0 =  (1 + cos(w0))/2;
    b1 = -(1 + cos(w0));
    b2 =  (1 + cos(w0))/2;
    a0 =   1 + alpha;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha;

%----------------------
%   Peaking Filter
%----------------------
elseif strcmp(type,'pkf')
    b0 =   1 + alpha*A;
    b1 =  -2*cos(w0);
    b2 =   1 - alpha*A;
    a0 =   1 + alpha/A;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha/A;

%----------------------
%   Band-pass Filter 1
%----------------------
% Constant skirt gain, peak gain = Q
elseif strcmp(type,'bp1')
    b0 =   sin(w0)/2;
    b1 =   0;
    b2 =  -sin(w0)/2;
    a0 =   1 + alpha;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha;

%----------------------
%   Band-pass Filter 2
%----------------------
% Constant 0 dB peak gain
elseif strcmp(type,'bp2')
    b0 =   alpha;
    b1 =   0;
    b2 =  -alpha;
    a0 =   1 + alpha;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha;

%----------------------
%    Notch Filter
%----------------------
elseif strcmp(type,'nch')
    b0 =   1;
    b1 =  -2*cos(w0);
    b2 =   1;
    a0 =   1 + alpha;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha;        

%----------------------
%    All-Pass Filter
%----------------------
elseif strcmp(type,'apf')
    b0 =   1 - alpha;
    b1 =  -2*cos(w0);
    b2 =   1 + alpha;
    a0 =   1 + alpha;
    a1 =  -2*cos(w0);
    a2 =   1 - alpha;

%----------------------
%    Low-Shelf Filter
%----------------------
elseif strcmp(type,'lsf')
    b0 = A*((A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha);
    b1 = 2*A*((A-1) - (A+1)*cos(w0));
    b2 = A*((A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha);
    a0 = (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha;
    a1 = -2*((A-1) + (A+1)*cos(w0));
    a2 = (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha;

%----------------------
%    High-Shelf Filter
%----------------------
elseif strcmp(type,'hsf')
    b0 = A*( (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha);
    b1 = -2*A*((A-1) + (A+1)*cos(w0));
    b2 = A*((A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha);
    a0 = (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha;
    a1 = 2*((A-1) - (A+1)*cos(w0));
    a2 = (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha;

% Otherwise, no filter
else 
    b0 = 1; a0 = 1;
    b1 = 0; b2 = 0; a1 = 0; a2 = 0;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Topology - Direct Form I, II, Transposed II
if (form == 1) % Direct Form I
    x2 = 0;    % Initial conditions 
    x1 = 0;
    y2 = 0;
    y1 = 0;
    for n = 1:N
        out(n,1) = (b0/a0)*in(n,1) + (b1/a0)*x1 + (b2/a0)*x2 ...
            + (-a1/a0)*y1 + (-a2/a0)*y2;
        x2 = x1;
        x1 = in(n,1);
        y2 = y1;
        y1 = out(n,1);
    end

elseif (form == 2) % Direct Form II
   w1 = 0;     % w1 & w2 are delayed versions of 'w'
   w2 = 0;
   for n = 1:N
       w = in(n,1) + (-a1/a0)*w1 + (-a2/a0)*w2;  
       out(n,1) = (b0/a0)*w + (b1/a0)*w1 + (b2/a0)*w2;
       w2 = w1;
       w1 = w;
   end

elseif (form == 3) % Transposed Direct Form II
   d1 = 0;    % d1 & d2 are outputs of the delay blocks
   d2 = 0;
   for n = 1:N
       out(n,1) = (b0/a0)*in(n,1) + d1;
       d1 = (b1/a0)*in(n,1) + (-a1/a0)*out(n,1) + d2;
       d2 = (b2/a0)*in(n,1) + (-a2/a0)*out(n,1); 
   end
   
else % No filtering  
   
    out = in;
    
end

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/biquadFilter.m -->

<!-- BEGIN_FILE: Ch_18/biquadStep.m -->
#### `Ch_18/biquadStep.m`

````matlab
% BIQUADSTEP
% This script demonstrates the result of taking the
% step response of a bi-quad LPF. Examples include
% changing the cut-off frequency and Q.
%
% See also BIQUADFILTER

clc;clear;close all;
% Input signal
Fs = 48000; Ts = 1/Fs;
x = ones(2*Fs,1); % Step Input 
N = length(x);
t = [0:N-1] * Ts; t = t(:);

% Changing the Cut-off Frequency
Q = 1.414;
dBGain = 0;
figure(1); hold on;
for freq = 1:4
     y = biquadFilter(x,Fs,freq,Q,dBGain,'lpf',1);
     plot(t,y);
end
hold off;legend('f = 1','f = 2','f = 3','f = 4');
xlabel('Time (sec.)');

% Changing the Bandwidth Q
freq = 1;
dBGain = 0;
figure(2); hold on;
for Q = 0.707/2:0.707/2:1.414
     y = biquadFilter(x,Fs,freq,Q,dBGain,'lpf',1);
     plot(t,y);
end
hold off;legend('Q = 0.3535','Q = 0.707','Q = 1.0605','Q = 1.414');
xlabel('Time (sec.)');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/biquadStep.m -->

<!-- BEGIN_FILE: Ch_18/compressor.m -->
#### `Ch_18/compressor.m`

````matlab
% COMPRESSOR
% This function implements a dynamic range compressor 
% with separate attack and release times. To visualize
% the results, code is included at the end of the function
% to plot the input signal and the compressed output signal.
% As a default, this code is commented so it does not create
% a plot each time the function is called.
%
% Input Variables
%   T : threshold relative to 0 dBFS
%   R : ratio (R to 1)
%   attackTime : units of seconds
%   releaseTime : units of seconds
% 
% See also COMPRESSOREXAMPLE, BASICCOMP

function [y] = compressor(x,Fs,T,R,attackTime,releaseTime)
N = length(x);
y = zeros(N,1);
lin_A = zeros(N,1);

% Initialize separate attack and release times
alphaA = exp(-log(9)/(Fs * attackTime));
alphaR = exp(-log(9)/(Fs * releaseTime));

gainSmoothPrev = 0; % Initialize smoothing variable

% Loop over each sample to see if it is above thresh
for n = 1:N
    % Turn the input signal into a uni-polar signal on the dB scale
    x_uni = abs(x(n,1));
    x_dB = 20*log10(x_uni/1);
    % Ensure there are no values of negative infinity
    if x_dB < -96
        x_dB = -96;
    end
    
    % Static Characteristics
    if x_dB > T
        gainSC = T + (x_dB - T)/R; % Perform Downwards Compression
    else 
        gainSC = x_dB; % Do not perform compression 
    end
          
    gainChange_dB = gainSC - x_dB;
    
    % smooth over the gainChange
    if gainChange_dB < gainSmoothPrev
        % attack mode
        gainSmooth = ((1-alphaA)*gainChange_dB) ...
            +(alphaA*gainSmoothPrev);
    else
        % release
        gainSmooth = ((1-alphaR)*gainChange_dB) ...
            +(alphaR*gainSmoothPrev);
    end

    % Convert to linear amplitude scalar
    lin_A(n,1) = 10^(gainSmooth/20);
    
    % Apply linear amplitude to input sample
    y(n,1) = lin_A(n,1) * x(n,1);
    
    % Update gainSmoothPrev used in the next sample of the loop
    gainSmoothPrev = gainSmooth;
end

% Uncomment for visualization
% t = [0:N-1]/Fs; t = t(:);
% subplot(2,1,1);
% plot(t,x); title('Input');axis([0 t(end) -1.1 1.1]); 
% subplot(2,1,2);
% plot(t,y,t,lin_A); title('Output'); axis([0 t(end) -1.1 1.1]);
% legend('Output Signal','Gain Reduction');


% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/compressor.m -->

<!-- BEGIN_FILE: Ch_18/compressorExample.m -->
#### `Ch_18/compressorExample.m`

````matlab
% COMPRESSOREXAMPLE
% This script creates a dynamic range compressor with
% separate attack and release times 
% 
% See also COMPRESSOR, BASICCOMP

% Acoustic Guitar "Audio" Sound file
[in,Fs] = audioread('AcGtr.wav');

% Parameters for Compressor
T = -15;   % Threshold = -15 dBFS
R = 10;     % Ratio = 10:1

% Initialize separate attack and release times
attackTime = 0.05;  % time in seconds
releaseTime = 0.25;  % time in seconds

% Compressor Function
out = compressor(in,Fs,T,R,attackTime,releaseTime);
sound(out,Fs);


% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/compressorExample.m -->

<!-- BEGIN_FILE: Ch_18/expander.m -->
#### `Ch_18/expander.m`

````matlab
% EXPANDER
% This function implements an expander/gate DR processor.
% A similar approach is used to a compressor, except
% the static characteristics are calculated differently.
%
% Input Variables
%   T : threshold relative to 0 dBFS
%   R : ratio (R to 1)
%   attackTime : units of seconds
%   releaseTime : units of seconds
%
% See also EXPANDER, COMPRESSOREXAMPLE
function [y] = expander(x,Fs,T,R,attackTime,releaseTime)
N = length(x);
y = zeros(N,1);
lin_A = zeros(N,1);

% Calculate separate attack and release times
alphaA = exp(-log(9)/(Fs * attackTime));
alphaR = exp(-log(9)/(Fs * releaseTime));

gainSmoothPrev = -144; % Initialize smoothing variable

% Loop over each sample to see if it is below thresh
for n = 1:N
    % Turn the input signal into a uni-polar signal on the dB scale
    x_uni = abs(x(n,1));
    x_dB = 20*log10(x_uni/1);
    % Ensure there are no values of negative infinity
    if x_dB < -144
        x_dB = -144;
    end
    
    % Static Characteristics
    if x_dB > T
        gainSC = x_dB; % Do not perform expansion
    else
        % Expander Calculation
        gainSC = T + (x_dB - T)*R ; % Perform Downwards Expansion
        
        % Gating (Use Instead of Expander)
        %gainSC = -144;
    end
          
    gainChange_dB = gainSC - x_dB;
    
    % smooth over the gainChange
    if gainChange_dB > gainSmoothPrev
        % attack mode
        gainSmooth = ((1-alphaA)*gainChange_dB) ...
            +(alphaA*gainSmoothPrev);
    else
        % release
        gainSmooth = ((1-alphaR)*gainChange_dB) ...
            +(alphaR*gainSmoothPrev);
    end

    % Convert to linear amplitude scalar
    lin_A(n,1) = 10^(gainSmooth/20);
    
    % Apply linear amplitude to input sample
    y(n,1) = lin_A(n,1) * x(n,1);
    
    % Update gainSmoothPrev used in the next sample of the loop
    gainSmoothPrev = gainSmooth;
end
t = [0:N-1]/Fs; t = t(:);

subplot(2,1,1);
plot(t,x); title('Input Signal');axis([0 14 -1.1 1.1]); 
subplot(2,1,2);
plot(t,y,t,lin_A); title('Output'); axis([0 14 -1.1 1.1]);
legend('Output Signal','Gain Reduction');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/expander.m -->

<!-- BEGIN_FILE: Ch_18/expanderExample.m -->
#### `Ch_18/expanderExample.m`

````matlab
% EXPANDEREXAMPLE
% This script demonstrates an expander/gate DR processor
%
% See also EXPANDER, COMPRESSOREXAMPLE
clear; clc; close all;

% Drums Sound file
[in,Fs] = audioread('monoDrums.wav');

% Parameters for Compressor
T = -20;   % Threshold = -20 dBFS
R = 3;     % Ratio = 3:1

% Initialize separate attack and release times
attackTime = 0.005;  % time in seconds
releaseTime = 0.4;  % time in seconds

out = expander(in,Fs,T,R,attackTime,releaseTime);
sound(out,Fs);
% sound(in,Fs); % For comparison

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/expanderExample.m -->

<!-- BEGIN_FILE: Ch_18/feedbackComp.m -->
#### `Ch_18/feedbackComp.m`

````matlab
% FEEDBACKCOMP
% This script creates a feed-back compressor.
% The processing of the detection path is similar
% to the feed-forward compressor. The main difference
% is the output, "y" is analyzed in the detection 
% path, not the input "x". A plot is produced
% at the end of the script to visualize the result.
%
% See also COMPRESSOR, BASICCOMP

% Acoustic Guitar "Audio" Sound file
[x,Fs] = audioread('AcGtr.wav');

% Parameters for Compressor
T = -15;   % Threshold = -15 dBFS
R = 10;     % Ratio = 10:1

% Initialize separate attack and release times
attackTime = 0.05;  % time in seconds
alphaA = exp(-log(9)/(Fs * attackTime));
releaseTime = 0.25;  % time in seconds
alphaR = exp(-log(9)/(Fs * releaseTime));

gainSmoothPrev = 0; % Initialize smoothing variable

y_prev = 0; % Initialize output for feed-back detection

N = length(x);
y = zeros(N,1);
lin_A = zeros(N,1);
% Loop over each sample to see if it is above thresh
for n = 1:N
    %%%%% Detection path based on the output signal, not "x"
    % Turn the input signal into a uni-polar signal on the dB scale
    y_uni = abs(y_prev);
    y_dB = 20*log10(y_uni/1);
    
    % Ensure there are no values of negative infinity
    if y_dB < -96
        y_dB = -96;
    end
    
    % Static Characteristics
    if y_dB > T
        gainSC = T + (y_dB - T)/R; % Perform Downwards Compression
    else 
        gainSC = y_dB; % Do not perform compression 
    end
          
    gainChange_dB = gainSC - y_dB;
    
    % smooth over the gainChange
    if gainChange_dB < gainSmoothPrev
        % attack mode
        gainSmooth = ((1-alphaA)*gainChange_dB) ...
            +(alphaA*gainSmoothPrev);
    else
        % release
        gainSmooth = ((1-alphaR)*gainChange_dB) ...
            +(alphaR*gainSmoothPrev);
    end

    % Convert to linear amplitude scalar
    lin_A(n,1) = 10^(gainSmooth/20);
    
    % Apply linear amplitude to input sample
    y(n,1) = lin_A(n,1) * x(n,1);
    y_prev = y(n,1); % Update for next cycle
    
    % Update gainSmoothPrev used in the next sample of the loop
    gainSmoothPrev = gainSmooth;
    
end
t = [0:N-1]/Fs; t = t(:);

subplot(2,1,1);
plot(t,x); title('Input Signal');axis([0 7 -1.1 1.1]); 
subplot(2,1,2);
plot(t,y,t,lin_A); title('Output'); axis([0 7 -1.1 1.1]);
legend('Output Signal','Gain Reduction');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/feedbackComp.m -->

<!-- BEGIN_FILE: Ch_18/rmsComp.m -->
#### `Ch_18/rmsComp.m`

````matlab
% RMSCOMP
% This script creates a compressor with 
% conventional RMS detection. The RMS value
% is calculated over a range of "M" samples.
% Note: attack and release are linked
%
% See also COMPRESSOR, RMSCOMP2
clear;clc;close all;

% Acoustic Guitar "Audio" Sound file
[x,Fs] = audioread('AcGtr.wav');

% Parameters for Compressor
T = -20;   % Threshold = -20 dBFS
R = 4;     % Ratio = 4:1

% Initialize separate attack and release times
attackTime = 0.1;  % time in seconds
alphaA = exp(-log(9)/(Fs * attackTime));
releaseTime = 0.25;  % time in seconds
alphaR = exp(-log(9)/(Fs * releaseTime));

gainSmoothPrev = 0; % Initialize smoothing variable

M = 2048; % length of RMS calculation

% Initialize the first time window in a buffer
x_win = [zeros(M/2,1);x(1:(M/2),1)];

N = length(x);
y = zeros(N,1);
lin_A = zeros(N,1);

% Loop over each sample to see if it is above thresh
for n = 1:N
    
    % Calculate the RMS for the current window
    x_rms = sqrt(mean(x_win.^2));
    
    % Turn the input signal into a uni-polar signal on the dB scale
    x_dB = 20*log10(x_rms);
    
    % Ensure there are no values of negative infinity
    if x_dB < -96
        x_dB = -96;
    end
    
    % Static Characteristics
    if x_dB > T
        gainSC = T + (x_dB - T)/R; % Perform compression
    else 
        gainSC = x_dB; % Do not perform compression 
    end
          
    gainChange_dB = gainSC - x_dB;
    
    % Convert to linear amplitude scalar
    lin_A(n,1) = 10^(gainChange_dB/20);
    
    % Apply linear amplitude to input sample
    y(n,1) = lin_A(n,1) * x(n,1);
    
    
    % Update the current time window
    if n+(M/2) < N
        x_win = [x_win(2:end,1) ; x(n+(M/2)+1,1)];
    else
        x_win = [x_win(2:end,1) ; 0];
    end
end
t = [0:N-1]/Fs; t = t(:);

subplot(2,1,1);
plot(t,x); title('Input Signal');axis([0 t(end) -1.1 1.1]); 
subplot(2,1,2);
plot(t,y,t,lin_A); title('Output'); axis([0 t(end) -1.1 1.1]);
legend('Output Signal','Gain Reduction');


% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/rmsComp.m -->

<!-- BEGIN_FILE: Ch_18/rmsComp2.m -->
#### `Ch_18/rmsComp2.m`

````matlab
% RMSCOMP2
% This script creates a compressor with approximated
% RMS detection. The RMS value is estimated
% by using feedback. Separate attack and release
% times can be achieved.
%
% See also RMSCOMP
clear;clc;close all;

% Acoustic Guitar "Audio" Sound file
[x,Fs] = audioread('AcGtr.wav');

% Parameters for Compressor
T = -12;   % Threshold = -12 dBFS
R = 4;     % Ratio = 4:1

% Initialize separate attack and release times
attackTime = 0.1;  % time in seconds
alphaA = exp(-log(9)/(Fs * attackTime));
releaseTime = 0.1;  % time in seconds
alphaR = exp(-log(9)/(Fs * releaseTime));

gainSmoothPrev = 0; % Initialize smoothing variable

N = length(x);
y = zeros(N,1);
lin_A = zeros(N,1);
% Loop over each sample to see if it is above thresh
for n = 1:N
    
    % Turn the input signal into a uni-polar signal on the dB scale
    x_dB = 20*log10(abs(x(n,1)));
    
    % Ensure there are no values of negative infinity
    if x_dB < -96
        x_dB = -96;
    end
    
    % Static Characteristics
    if x_dB > T
        gainSC = T + (x_dB - T)/R; % Perform Downwards Compression
    else 
        gainSC = x_dB; % Do not perform compression 
    end
          
    gainChange_dB = gainSC - x_dB;
    
    % smooth over the gainChange
    if gainChange_dB < gainSmoothPrev
        % attack mode
        gainSmooth = -sqrt(((1-alphaA)*gainChange_dB^2) ...
            +(alphaA*gainSmoothPrev^2));
    else
        % release
        gainSmooth = -sqrt(((1-alphaR)*gainChange_dB^2) ...
            +(alphaR*gainSmoothPrev^2));
    end

    % Convert to linear amplitude scalar
    lin_A(n,1) = 10^(gainSmooth/20);
    
    % Apply linear amplitude to input sample
    y(n,1) = lin_A(n,1) * x(n,1);
    
    % Update gainSmoothPrev used in the next sample of the loop
    gainSmoothPrev = gainSmooth;
   
end
t = [0:N-1]/Fs; t = t(:);

subplot(2,1,1);
plot(t,x); title('Input Signal');axis([0 t(end) -1.1 1.1]); 
subplot(2,1,2);
plot(t,y,t,lin_A); title('Output'); axis([0 t(end) -1.1 1.1]);
legend('Output Signal','Gain Reduction');


% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/rmsComp2.m -->

<!-- BEGIN_FILE: Ch_18/sidechainComp.m -->
#### `Ch_18/sidechainComp.m`

````matlab
% SIDECHAINCOMP
% This script creates a side-chain compressor with
% a synthesizer signal and kick drum signal. 
%
% See also COMPRESSOR
clear;clc;
% Synthesizer Input Signal
[x,Fs] = audioread('Synth.wav');
% Kick Drum for the Detection Path
[sc] = audioread('Kick.wav');

% Parameters for Compressor
T = -24;   % Threshold = -24 dBFS
R = 10;     % Ratio = 10:1

% Initialize separate attack and release times
attackTime = 0.05;  % time in seconds
alphaA = exp(-log(9)/(Fs * attackTime));
releaseTime = 0.25;  % time in seconds
alphaR = exp(-log(9)/(Fs * releaseTime));

gainSmoothPrev = 0; % Initialize smoothing variable

N = length(sc);
y = zeros(N,1);
lin_A = zeros(N,1);
% Loop over each sample to see if it is above thresh
for n = 1:N
    %%%%% Detection path based on the Kick Drum input signal
    % Turn the input signal into a uni-polar signal on the dB scale
    sc_uni = abs(sc(n,1));
    sc_dB = 20*log10(sc_uni/1);
    % Ensure there are no values of negative infinity
    if sc_dB < -96
        sc_dB = -96;
    end
    
    % Static Characteristics
    if sc_dB > T
        gainSC = T + (sc_dB - T)/R; % Perform Downwards Compression
    else 
        gainSC = sc_dB; % Do not perform compression 
    end
          
    gainChange_dB = gainSC - sc_dB;
    
    % smooth over the gainChange
    if gainChange_dB < gainSmoothPrev
        % attack mode
        gainSmooth = ((1-alphaA)*gainChange_dB) ...
            +(alphaA*gainSmoothPrev);
    else
        % release
        gainSmooth = ((1-alphaR)*gainChange_dB) ...
            +(alphaR*gainSmoothPrev);
    end

    % Convert to linear amplitude scalar
    lin_A(n,1) = 10^(gainSmooth/20);
    
    % Apply linear amplitude to synthesizer input sample
    y(n,1) = lin_A(n,1) * x(n,1);
    
    % Update gainSmoothPrev used in the next sample of the loop
    gainSmoothPrev = gainSmooth;
end
t = [0:N-1]/Fs; t = t(:);

subplot(3,1,1);
plot(t,x); title('Input Signal - Synth');axis([0 t(end) -1.1 1.1]); 
subplot(3,1,2);
plot(t,sc); title('Sidechain - Kick');axis([0 t(end) -1.1 1.1]);
subplot(3,1,3);
plot(t,y,t,lin_A); title('Output'); axis([0 t(end) -1.1 1.1]);
legend('Output Signal','Gain Reduction');

sound(y,Fs);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/sidechainComp.m -->

<!-- BEGIN_FILE: Ch_18/stepDemo.m -->
#### `Ch_18/stepDemo.m`

````matlab
% STEPDEMO
% This script demonstrates the process of measuring
% the step response of a 1st-order, feed-back LPF.
% A plot is created showing a comparison between
% the input step signal and the output response.

% Initialize the Sampling Rate
Fs = 48000; Ts = 1/Fs;

% Create Step Input Signal
x = [zeros(Fs,1) ; ones(Fs,1)];
N = length(x);
% Initialize gain value
alpha = 0.9995;  % Also try values between 0.999-0.9999
q = 0; % Initialize feedback variable
for n = 1:N
    y(n,1) = (1-alpha) * x(n,1) + alpha * q;
    q = y(n,1); % Stores the "previous" value for the next loop cycle
end

t = [0:N-1]*Ts; % Time vector for plot
plot(t,x,t,y);
axis([0 2 -0.1 1.1]); xlabel('Time (sec.)');
legend('Step Input','Output');

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/stepDemo.m -->

<!-- BEGIN_FILE: Ch_18/stepDesign.m -->
#### `Ch_18/stepDesign.m`

````matlab
% STEPDESIGN
% This function can be used to design a second-order
% system with specified step response characteristics.
%
% Input Variables
%   Fs : sampling rate
%   OS : percent overshoot
%   T : time in seconds of "characteristic"
%   characteristic : 'pk' (peak) or 'ss' (settling time)

function [b,a] = stepDesign(Fs,OS,T,characteristic)

if OS < 0.00001 % Ensure a minimum value of Overshoot
    OS = 0.00001;
end

% Convert Percent OverShoot to Damping
L = -log(OS/100)/(sqrt(pi^2 + (log(OS/100))^2));

% Find 'wn' - undamped natural frequency
% based on characteristic type 
if strcmp('pk',characteristic) 
    % Peak Time
    wn = pi/(T*sqrt(1-L^2));

elseif strcmp('st',characteristic) 
    %Setting Time (0.02 of steady-state)
    wn = -log(0.02*sqrt(1-L^2))/(L*T);
else
   % Return invalid type 
   disp('Please enter a chacteristic, "pk"-peak, "st"-settling time');
   return;
end

% Continuous Filter:
%
%                (wn)^2
% H(s) = ------------------------
%         s^2 + 2*L*wn + (wn)^2
%

num = wn^2;
den = [1 2*L*wn wn^2];

% Perform bilinear transform on continuous system
% to find discrete system
[b,a] = bilinear(num,den,Fs);

% Plot the step response
%n = 2*Fs; % number of seconds
%[h,t] = stepz(b,a,n,Fs);
%plot(t,h);
%phi = atan(L/(sqrt(1-L^2)));
%yStep = 1- exp(-L*wn*t) .* cos(wn*sqrt(1-L^2)*t - phi)/(sqrt(1-L^2));
%figure;
%plot(t,yStep);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/stepDesign.m -->

<!-- BEGIN_FILE: Ch_18/stepDesignExample.m -->
#### `Ch_18/stepDesignExample.m`

````matlab
% STEPDESIGNEXAMPLE
% This script demonstrates the stepDesign function
% for designing a second-order system with
% specified characteristics. The system is designed
% based on the percent overshoot, setttling time, 
% and peak time.
%
% See also STEPDESIGN
clc;clear;close all;

Fs = 48000;

% Example - filter design based on settling time
OS = 20;    % Percent overshoot
% Settling time in seconds
ts = .25;   % (within 2% of steady-state)
[b,a] = stepDesign(Fs,OS,ts,'st');
n = 1*Fs;  % number of seconds for step response
[h,t] = stepz(b,a,n,Fs);
plot(t,h);

figure;

% Example - filter design based on peak time
OS = 10;   % Percent overshoot
tp = 0.75; % Peak time in seconds
[b,a] = stepDesign(Fs,OS,tp,'pk');
n = 2*Fs; % number of seconds for step response
[h,t] = stepz(b,a,n,Fs);
plot(t,h);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/stepDesignExample.m -->

<!-- BEGIN_FILE: Ch_18/stepResponse.m -->
#### `Ch_18/stepResponse.m`

````matlab
% STEPRESPONSE
% This script demonstrates the built-in,
% MATLAB function: stepz(b,a). The step 
% response for several first-order systems
% is compared using different feed-back gains.
%
% See also STEPZ

Fs = 48000; % Initialize the Sampling Rate
sec = 1;   % Time length in seconds
n = sec*Fs; % Convert seconds to # of samples

% Define different gain values to test
gains = [0.999;0.9995;0.9997;0.9999];

% Determine a new step response each time through the loop
for element = 1:length(gains) 
    alpha = gains(element,1);
    b = [(1-alpha)];
    a = [1 , -alpha];
    [h,t] = stepz(b,a,n,Fs);
    plot(t,h);hold on;
end

hold off; axis([0 sec -0.1 1.1]); xlabel('Time (sec.)');
AX = legend('.9990','.9995','.9997','.9999');set(AX,'FontSize',12);

% This source code is provided without any warranties as published in 
% "Hack Audio: An Introduction to Computer Programming and Digital Signal
% Processing in MATLAB" � 2019 Taylor & Francis.
% 
% It may be used for educational purposes, but not for commercial 
% applications, without further permission.
%
% Book available here (uncomment):
% url = ['https://www.routledge.com/Hack-Audio-An-Introduction-to-'...
% 'Computer-Programming-and-Digital-Signal/Tarr/p/book/9781138497559'];
% web(url,'-browser');
% 
% Companion website resources (uncomment):
% url = 'http://www.hackaudio.com'; 
% web(url,'-browser');
````
<!-- END_FILE: Ch_18/stepResponse.m -->

<!-- END_CHAPTER: Ch_18 -->
