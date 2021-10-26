import numpy as np
import pandas as pd
import sqlite3
import glob
import sys
sys.path.insert(0, "/mnt/ssd1/Documents/Vivek/MalkoFish/MalkoAnalyzer")
import swarm
import BaseToolbox as bt
import plotBox as pb
import plotter
import copy
from imp import reload
reload(pb)
reload(plotter)
reload(bt)

plotter.pb.boxSize.defineBoxSize((-5,5),(-5,5),(-1,1)) 
plotter.pb.boxSizeRelative.defineBoxSize((-10,10),(-10,10),(-1,1))

def rotate(xy, radians):
    x, y = xy
    c, s = np.cos(radians), np.sin(radians)
    j = np.matrix([[c, s], [-s, c]])
    m = np.dot(j, [x, y])

    return m

def preprocess_fly_data(dataDIR, projectDB, expDB, nPosts):
    # establish a connecttion to the project database
    conn = sqlite3.connect(projectDB)
    # connect a cursor that goes through the project database
    cursorProject = conn.cursor()
    # establish a second connecttion to the experiment database
    conn2 = sqlite3.connect(expDB)
    # connect a cursor that goes through the experiment database
    cursorExperiment = conn2.cursor()

    # pick experiments from specified project
    cursorExperiment.execute("Select expId from experiments where project = ? and exp >= ? and exp < ?",('DecisionGeometry',  (nPosts-2)*10, (nPosts-1)*10))
    fetched = cursorExperiment.fetchall()
    print('fetched : ' + str(fetched))

    # stack dataframes in current directory
    angles = []
    dataDict0 = []
    dataDict1 = []
    dataDict2 = []
    rot_post0 = []
    for uuid in range(0,len(fetched)):
        cursorExperiment.execute("Select exp from experiments where expId = ?", (fetched[uuid][0],))
        fetch_exp = cursorExperiment.fetchall()[0][0]
        cursorExperiment.execute("Select replicate from experiments where expId = ?", (fetched[uuid][0],))
        fetch_rep = cursorExperiment.fetchall()[0][0]

        cursorProject.execute("Select post0 from projects where project = ? and exp = ? and replicate = ?",('DecisionGeometry', fetch_exp, fetch_rep))
        dataDict0.append(cursorProject.fetchall())
        cursorProject.execute("Select post1 from projects where project = ? and exp = ? and replicate = ?",('DecisionGeometry', fetch_exp, fetch_rep))
        dataDict1.append(cursorProject.fetchall())
        cursorProject.execute("Select post2 from projects where project = ? and exp = ? and replicate = ?",('DecisionGeometry', fetch_exp, fetch_rep))
        dataDict2.append(cursorProject.fetchall())
        
        tmp = pd.read_csv(dataDIR + fetched[uuid][0] + "/results.csv", names = ["x", "y", "z", "dir", "event", "t", "nStimuli"])
        tmp['uuid'] = uuid
        tmp['post0_x'] = tmp['post0_y'] = tmp['post1_x'] = tmp['post1_y'] = np.nan
        if nPosts == 3:
            tmp['post2_x'] = tmp['post2_y'] = np.nan
        
        df = tmp if uuid == 0 else pd.concat([df,tmp])
        rot_post0.append([eval(dataDict0[uuid][1][0])['distance'], 0.0])

        cursorProject.execute("Select post1 from projects where project = ? and exp >= ? and exp < ?",('DecisionGeometry', (nPosts-2)*10, (nPosts-1)*10))
        for a in np.unique(cursorProject.fetchall()):
            if a != 'None' and eval(a)['angle'] not in angles:
                angles.append(eval(a)['angle'])

    angles = np.sort(np.array(angles))

    # add post positions to dataframe
    nStimuli = np.unique(df['nStimuli'])
    for uuid in range(0,len(fetched)):
        for i in range(0,np.max(nStimuli)+1):
            df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'post0_x'] = eval(dataDict0[uuid][nStimuli[i]][0])['position'][0]
            df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'post0_y'] = eval(dataDict0[uuid][nStimuli[i]][0])['position'][1]
            if i > 0 and i < np.max(nStimuli):
                df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'post1_x'] = eval(dataDict1[uuid][nStimuli[i]][0])['position'][0]
                df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'post1_y'] = eval(dataDict1[uuid][nStimuli[i]][0])['position'][1]
                if nPosts == 3:
                    df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'post2_x'] = eval(dataDict2[uuid][nStimuli[i]][0])['position'][0]
                    df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'post2_y'] = eval(dataDict2[uuid][nStimuli[i]][0])['position'][1]

    # initialise rotation of tracks and posts
    df['rotated_x'] = df['rotated_y'] = 0.0
    df['rotated_post0_x'] = df['rotated_post0_y'] = df['rotated_post1_x'] = df['rotated_post1_y'] = 0.0
    if nPosts == 3:
        df['rotated_post2_x'] = df['rotated_post2_y'] = 0.0
    df['ang'] = 0.0
    prerot_idx = 7+2*nPosts
    postrot_idx = 7+2*nPosts+2+2*nPosts

    # get rotation angle
    for uuid in range(0,len(fetched)):
        rot_post0_x = rot_post0[uuid][0]
        rot_post0_y = rot_post0[uuid][1]
        df.iloc[np.where(df['uuid'] == uuid)[0],postrot_idx+1] = np.arctan2(df.iloc[np.where(df['uuid'] == uuid)[0],9], df.iloc[np.where(df['uuid'] == uuid)[0],8])

    # rotate tracks and posts (posts are now centred along positive x-axis)
    for uuid in range(0,len(fetched)):
        for i in range(0,np.max(nStimuli)+1):
            ang = np.unique(df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'ang'])[0] 
            if i > 0 and i < np.max(nStimuli): 
                ang += (nPosts-1)*eval(dataDict0[uuid][nStimuli[i]][0])['angle']/2
                
            rx,ry = rotate(np.array((df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'x'],df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'y'])), ang)
            df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'rotated_x'] = np.squeeze(np.asarray(rx.T))
            df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'rotated_y'] = np.squeeze(np.asarray(ry.T))
            
            rp0x,rp0y = rotate(np.array((df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'post0_x'],df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'post0_y'])), ang)
            df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'rotated_post0_x'] = np.squeeze(np.asarray(rp0x.T))
            df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'rotated_post0_y'] = np.squeeze(np.asarray(rp0y.T))
            
            rp1x,rp1y = rotate(np.array((df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'post1_x'],df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'post1_y'])), ang)
            df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'rotated_post1_x'] = np.squeeze(np.asarray(rp1x.T))
            df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'rotated_post1_y'] = np.squeeze(np.asarray(rp1y.T))
            
            if nPosts == 3:
                rp2x,rp2y = rotate(np.array((df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'post2_x'],df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'post2_y'])), ang)
                df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'rotated_post2_x'] = np.squeeze(np.asarray(rp2x.T))
                df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == i),'rotated_post2_y'] = np.squeeze(np.asarray(rp2y.T))
          
    return df, fetched, angles

def distance_filter_trajectories(distance, nPosts, df):
	p0_dist = np.sqrt((df['x'] - df['post0_x'])**2 + (df['y'] - df['post0_y'])**2)
	p1_dist = np.sqrt((df['x'] - df['post1_x'])**2 + (df['y'] - df['post1_y'])**2)
	if nPosts == 2:
		df['dmin'] = np.nanmin([p0_dist, p1_dist], axis=0)
	else:
		p2_dist = np.sqrt((df['x'] - df['post2_x'])**2 + (df['y'] - df['post2_y'])**2)
		df['dmin'] = np.nanmin([p0_dist, p1_dist, p2_dist], axis=0)

	tmax = df.loc[:,['uuid', 'nStimuli', 'event', 't']]
	tmax = tmax.groupby(['uuid', 'nStimuli', 'event']).max().reset_index()
	dmin = df.loc[:,['uuid', 'nStimuli', 'event', 't', 'dmin']]

	dists = pd.merge(tmax, dmin, how='left')
	dists = dists[dists['dmin'] < distance]
	dists = dists.loc[:,['uuid', 'nStimuli', 'event']]

	df = pd.merge(dists, df, how='left')

	for i1, u in enumerate(np.unique(df['uuid'])):
	    tmp = df[df['uuid'] == u]
	    for i2, n in enumerate(np.unique(tmp['nStimuli'])):
	        tmp2 = tmp[tmp['nStimuli'] == n]
	        for i3, e in enumerate(np.unique(tmp2['event'])):
	            df.loc[(df['uuid'] == u) & (df['nStimuli'] == n) & (df['event'] == e), 'uuid'] = i1
	            df.loc[(df['uuid'] == u) & (df['nStimuli'] == n) & (df['event'] == e), 'nStimuli'] = i2
	            df.loc[(df['uuid'] == u) & (df['nStimuli'] == n) & (df['event'] == e), 'event'] = i3

	return df

def get_malko_architecture(projectDB, expDB, nPosts, df, fetched, angles):
    # establish a connecttion to the project database
    conn = sqlite3.connect(projectDB)
    # connect a cursor that goes through the project database
    cursorProject = conn.cursor()
    # establish a second connecttion to the experiment database
    conn2 = sqlite3.connect(expDB)
    # connect a cursor that goes through the experiment database
    cursorExperiment = conn2.cursor()


    x = df[['rotated_x','rotated_y']].values
    x = np.c_[x,x[:,0]*0]
    x_Post0 = df[['rotated_post0_x', 'rotated_post0_y']].values
    x_Post0 = np.c_[x_Post0,x_Post0[:,0]*0]
    x_Post1 = df[['rotated_post1_x', 'rotated_post1_y']].values
    x_Post1 = np.c_[x_Post1,x_Post1[:,0]*0]
    if nPosts == 3:
        x_Post2 = df[['rotated_post2_x', 'rotated_post2_y']].values
        x_Post2 = np.c_[x_Post2,x_Post2[:,0]*0]
    t = df['t'].values
    uuids = df['uuid'].values
    nStimuli = df['nStimuli'].values
    events = df['event'].values
    print(x.shape)

    massive_experiment = swarm.Swarm()
    for i in range(0, len(angles)+2):
        massive_experiment.addFish(swarm.Swarm())

    for uuid in range(0,len(fetched)):
        cursorExperiment.execute("Select exp from experiments where expID = ?",(fetched[uuid][0],))
        exp = cursorExperiment.fetchall()
        nStim = len(np.unique(df.loc[df['uuid'] == uuid,'nStimuli']))
        for n in range(0,nStim):
            stimuli = swarm.Swarm()
            nEvents = len(np.unique(df.loc[(df['uuid'] == uuid) & (df['nStimuli'] == n),'event']))
            for e in range(0,nEvents):
                event = swarm.Swarm()
                idx = np.where((uuids == uuid) & (nStimuli == n) & (events == e))[0]
                if len(idx) > 30:
                    event.addFish(swarm.Fish(x, t, idx=idx, burst=False, fishId=0, virtual=False))
                    event.addFish(swarm.Fish(x_Post0, t, idx=idx, burst=False, fishId=1, virtual=True))
                    if n != 0 and n != np.max(nStimuli):
                        event.addFish(swarm.Fish(x_Post1, t, idx=idx, burst=False, fishId=2, virtual=True))
                        if nPosts > 2:
                            event.addFish(swarm.Fish(x_Post2, t, idx=idx, burst=False, fishId=3, virtual=True))
                    event.fishReferential()
                    stimuli.addFish(event)
            if n == 0:
                massive_experiment.swarm[n].addFish(stimuli)
            elif n == np.max(nStimuli):
                massive_experiment.swarm[len(angles)+1].addFish(stimuli)
            else:
                cursorExperiment.execute("Select replicate from experiments where expID = ?",(fetched[uuid][0],))
                rep = cursorExperiment.fetchall()
                cursorProject.execute("Select post1 from projects where project = ? and exp = ? and replicate = ? and nStimuli = ?",('DecisionGeometry', exp[0][0], rep[0][0], n))
                post = cursorProject.fetchall()
                s_number = np.where(angles == eval(post[0][0])['angle'])[0][0]
                massive_experiment.swarm[s_number+1].addFish(stimuli)
                
    for i in range(0, len(angles)+2):
        massive_experiment.swarm[i].fillMetaFish()

    return massive_experiment