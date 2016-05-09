#!/usr/bin/python3.2
# coding: utf-8

from wave import open as open_wave
import numpy as np
from filter import ofb
import wave
#import matplotlib.pyplot as plt
#import glob
#from struct import pack
#import scipy.io.wavfile
from wave import open as open_wave

maskFile = input("Please choose a mask sound from the following: ocean, creek, woods, rain \n")

#for filename in glob.glob('*.wav'):
#myAudioLeft, myAudioRight = thinkdsp.read_wave("ocean.wav")
fp = open_wave("../mask/wav/unfiltered/"+maskFile+".wav", 'r')

z_str = fp.readframes(fp.getnframes())
fp.close()
ys = np.fromstring(z_str, dtype=np.int16)

# if it's in stereo, just pull out the first channel
myAudioLeft = ys[::2]
myAudioRight = ys[1::2]


audio_len = myAudioLeft.size
fs = 44100
fade_len = 3*fs
#maxVol=2**15-1.0
kaiser = np.kaiser(2*fade_len, 5)

#cross kaiser with original wave
for i in range (0,fade_len-1):
    myAudioLeft[i] *= kaiser[i]
    myAudioLeft[audio_len - fade_len + i] *= kaiser[fade_len + i]
    myAudioRight[i] *= kaiser[i]
    myAudioRight[audio_len - fade_len + i] *= kaiser[fade_len + i]
    
# write 1-period crossfade

for i in range(0,fade_len - 1):
    myAudioLeft[i] += myAudioLeft[audio_len - fade_len + i]
    myAudioLeft[audio_len - fade_len + i] = myAudioLeft[i]
    myAudioRight[i] += myAudioRight[audio_len - fade_len + i]
    myAudioRight[audio_len - fade_len + i] = myAudioRight[i]

#myAudioLeft = (maxVol*myAudioLeft)
#myAudioRight = (maxVol*myAudioRight)

#my_ys = np.vstack((my_ys_left,my_ys_right))
    
#wvData = my_ys.T.astype('int16').tostring()

#wv = wave.open('crossfaded wave samples/1period_stereo.wav', 'wb')
#wv.setparams((2, 2, fs, audio_len, 'NONE', 'not compressed'))

#wv.writeframes(wvData)
#wv.close()

yLeft , states = ofb.filter(myAudioLeft,ffilt=True, states=None)
yRight , states = ofb.filter(myAudioRight,ffilt=True, states=None)

#reading frequency response of each band
#w, h = signal.freqz(myAudioLeft.ys)
#w1, h1= signal.freqz(oneL)
#w2, h2= signal.freqz(twoL)
#w3, h3= signal.freqz(threeL)
#w4, h4= signal.freqz(fourL)
#w5, h5= signal.freqz(fiveL)
#w6, h6= signal.freqz(sixL)
#w7, h7= signal.freqz(sevenL)
#w8, h8= signal.freqz(eightL)
#w9, h9= signal.freqz(nineL)
#w10, h10= signal.freqz(tenL)

#plotting frequency responses
#fig, ax1 = plt.subplots()

#ax1.plot(w1, 20 * np.log10(abs(h1)),'#a6405f')
#ax1.twinx().plot(w1, 20 * np.log10(abs(h2)),'#b2d6c5')
#ax1.twinx().plot(w1, 20 * np.log10(abs(h3)),'#70968d')
#ax1.twinx().plot(w1, 20 * np.log10(abs(h4)),'#54596b')
#ax1.twinx().plot(w1, 20 * np.log10(abs(h5)),'#4f414a')
#ax1.twinx().plot(w1, 20 * np.log10(abs(h6)),'#a6405f')
#ax1.twinx().plot(w1, 20 * np.log10(abs(h7)),'#b2d6c5')
#ax1.twinx().plot(w1, 20 * np.log10(abs(h8)),'#70968d')
#ax1.twinx().plot(w1, 20 * np.log10(abs(h9)),'#54596b')
#ax1.twinx().plot(w1, 20 * np.log10(abs(h10)),'#4f414a')

#frequency response of the original oceanwave
#ax1.plot(w, 20 * (np.angle(h)),'#000000')

#reconstructing the oceanwave (synthesis)
#myYsL = ((oneL)+(twoL)+(threeL)+(fourL)+(fiveL)+(sixL)+(sevenL)+(eightL)+(nineL)+(tenL))
#myYsL = yLeft.sum(axis = 1)
#myYsR = ((oneR)+(twoR)+(threeR)+(fourR)+(fiveR)+(sixR)+(sevenR)+(eightR)+(nineR)+(tenR))
#myYsR = yRight.sum(axis = 1)

#myYs = np.vstack((myYsL,myYsR))
#wr, hr = signal.freqz(myYsL)
#ax1.twinx().plot(wr, 20 * (np.angle(hr)),'#FF0000')

#plt.show()

#wvData = myYs.T.astype('int16').tostring()
#wv = wave.open('../python/reconstructed mask/reconstructed.wav', 'wb')
#wv.setparams((2, 2, fs, audio_len, 'NONE', 'not compressed'))
#wv.writeframes(wvData)
#wv.close()
one = np.vstack((yLeft[:,0],yRight[:,0]))
wvData = one.astype('int16').tostring()
wv = wave.open('../mask/wav/filtered/currently_playing/b0.wav', 'wb')
wv.setparams((2, 2, fs, audio_len, 'NONE', 'not compressed'))
wv.writeframes(wvData)
wv.close()
two = np.vstack((yLeft[:,1],yRight[:,1]))
wvData = two.astype('int16').tostring()
wv = wave.open('../mask/wav/filtered/currently_playing/b1.wav', 'wb')
wv.setparams((2, 2, fs, audio_len, 'NONE', 'not compressed'))
wv.writeframes(wvData)
wv.close()
three = np.vstack((yLeft[:,2],yRight[:,2]))
wvData = three.astype('int16').tostring()
wv = wave.open('../mask/wav/filtered/currently_playing/b2.wav', 'wb')
wv.setparams((2, 2, fs, audio_len, 'NONE', 'not compressed'))
wv.writeframes(wvData)
wv.close()    
four = np.vstack((yLeft[:,3],yRight[:,3]))
wvData = four.astype('int16').tostring()
wv = wave.open('../mask/wav/filtered/currently_playing/b3.wav', 'wb')
wv.setparams((2, 2, fs, audio_len, 'NONE', 'not compressed'))
wv.writeframes(wvData)
wv.close()
five = np.vstack((yLeft[:,4],yRight[:,4]))
wvData = five.astype('int16').tostring()
wv = wave.open('../mask/wav/filtered/currently_playing/b4.wav', 'wb')
wv.setparams((2, 2, fs, audio_len, 'NONE', 'not compressed'))
wv.writeframes(wvData)
wv.close()
six = np.vstack((yLeft[:,5],yRight[:,5]))
wvData = six.astype('int16').tostring()
wv = wave.open('../mask/wav/filtered/currently_playing/b5.wav', 'wb')
wv.setparams((2, 2, fs, audio_len, 'NONE', 'not compressed'))
wv.writeframes(wvData)
wv.close()
seven = np.vstack((yLeft[:,6],yRight[:,6]))
wvData = seven.astype('int16').tostring()
wv = wave.open('../mask/wav/filtered/currently_playing/b6.wav', 'wb')
wv.setparams((2, 2, fs, audio_len, 'NONE', 'not compressed'))
wv.writeframes(wvData)
wv.close()
eight = np.vstack((yLeft[:,7],yRight[:,7]))
wvData = eight.astype('int16').tostring()
wv = wave.open('../mask/wav/filtered/currently_playing/b7.wav', 'wb')
wv.setparams((2, 2, fs, audio_len, 'NONE', 'not compressed'))
wv.writeframes(wvData)
wv.close()
nine = np.vstack((yLeft[:,8],yRight[:,8]))
wvData = nine.astype('int16').tostring()
wv = wave.open('../mask/wav/filtered/currently_playing/b8.wav', 'wb')
wv.setparams((2, 2, fs, audio_len, 'NONE', 'not compressed'))
wv.writeframes(wvData)
wv.close()
ten = np.vstack((yLeft[:,9],yRight[:,9]))
wvData = ten.astype('int16').tostring()
wv = wave.open('../mask/wav/filtered/currently_playing/b9.wav', 'wb')
wv.setparams((2, 2, fs, audio_len, 'NONE', 'not compressed'))
wv.writeframes(wvData)
wv.close()
