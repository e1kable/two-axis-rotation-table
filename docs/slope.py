# -*- coding: utf-8 -*-
"""
Created on Tue Nov 14 09:18:49 2023

@author: fischerg
"""

import numpy as np
import matplotlib.pyplot as plt


def slope(t, ts, phidot_max, phi):

    phi_slope = phidot_max*ts
    if phi_slope > phi:

        t0 = np.sqrt(phi/2 * 2*ts / phiDotMax)

        phiDotEnd = phidot_max / ts * t0

        t1 = phiDotEnd*ts/phidot_max + t0

        if t < t0:
            return phidot_max / ts * t
        elif t >= t0 and t < t1:
            return -phidot_max / ts * (t-t0) + phiDotEnd
        else:
            return 0
        pass
    else:
        tg = phi/phidot_max-ts
        te = ts+tg
        if t < ts:
            return phidot_max / ts * t
        elif t >= ts and t < te:
            return phidot_max
        elif t >= te and t < te+ts:
            return phidot_max * (-1 / ts * t + 1 + 1 / ts*te)
        else:
            return 0


def positionSlope(t, ts, phiDotMax, phi):

    phi_slope = phiDotMax*ts
    if phi_slope > phi:

        t0 = np.sqrt(phi/2 * 2*ts / phiDotMax)
        phiDotEnd = phiDotMax / ts * t0

        t1 = phiDotEnd*ts/phiDotMax + t0

        if t < t0:
            return (1/2)*(phiDotMax/ts)*t**2
        elif t >= t0 and t < t1:
            return phi/2 - (phiDotMax/(2*ts))*(t-t0)**2 + phiDotEnd*(t-t0)
        else:
            return phi

    else:
        phi_rest = phi-phi_slope
        tr = phi_rest/phiDotMax

        if t <= ts:
            return (1/2)*(phiDotMax/ts)*t**2

        if t > ts and t <= ts+tr:
            return phiDotMax*(ts/2 + t - ts)

        elif t > ts+tr and t <= 2*ts+tr:
            return phiDotMax*(ts + tr) - (1/2)*(phiDotMax/ts)*(-t+tr+2*ts)**2

        else:
            return phiDotMax*(ts+tr)


def getTbyPhi(phi_i, ts, phiDotMax, phi):

    phi_slope = phiDotMax*ts

    if phi_slope > phi:

        t0 = np.sqrt(phi/2 * 2*ts / phiDotMax)
        phiDotEnd = phiDotMax / ts * t0

        t1 = phiDotEnd*ts/phiDotMax + t0

        if phi_i < phi/2:
            return np.sqrt(2*phi_i*ts/phiDotMax)

        elif phi_i >= phi/2 and phi_i < phi:
            ti = ts*phiDotEnd/phiDotMax - \
                np.sqrt((ts*phiDotEnd/phiDotMax)**2 -
                        2*ts*(phi_i-phi/2)/phiDotMax)
            return t0+ti
        else:
            return t0+t1

    else:

        phi_rest = phi-phi_slope
        tr = phi_rest/phiDotMax

        if phi_i < phiDotMax*(ts/2):
            return np.sqrt(phi_i * 2*ts / phiDotMax)

        elif phi_i >= phiDotMax*(ts/2) and phi_i <= phiDotMax*(ts/2 + tr):

            return ts + (phi_i-phiDotMax*(ts/2))/phiDotMax

        elif phi_i > phiDotMax*(ts/2 + tr) and phi <= phiDotMax*(ts+tr):
            return -np.sqrt((-phi_i+phiDotMax*(ts+tr))/((1/2)*phiDotMax/ts)) + tr + 2*ts
        else:
            return 2*ts+tr


phiDotMax = 20
ts = 2
phiPerStep = 360/1600

phi = 40


# generate velocity curve

ttot = getTbyPhi(phi, ts, phiDotMax, phi)

N = int(1e4)
dt = ttot/N
t = np.arange(N)*dt

s = np.array([slope(ti, ts, phiDotMax, phi) for ti in t])


s_int = np.array([positionSlope(ti, ts, phiDotMax, phi) for ti in t])

phis = np.linspace(0, phi, 1000)
t_by_phi = np.array([getTbyPhi(pi, ts, phiDotMax, phi) for pi in phis])

# Generate steps from it
Nsteps = int(np.round(phi/phiPerStep))

phiProx = Nsteps*phiPerStep

print(f"Nsteps: {Nsteps}, phiProx:{phiProx}")

stepTimes = np.array([getTbyPhi(pi, ts, phiDotMax, phi)
                     for pi in np.arange(Nsteps)*phiPerStep])


fig, ax = plt.subplots(3, 1)
ax[0].plot(t, s)
ax2 = ax[0].twinx()
ax2.plot(t, s_int, color='r')
ax2.axhline(phi, color='k', linestyle=':')

ax[1].plot(t_by_phi, phis)
ax[1].axhline(phi, color='k', linestyle=':')

ax[2].stem(stepTimes, np.ones(Nsteps)*phiPerStep)
ax2 = ax[2].twinx()
ax2.plot(stepTimes, np.cumsum(np.arange(Nsteps))*phiPerStep, color='r')
