#!/usr/bin/env python3

# unzip
# preprocess
# generate selection

from threading import Thread
import argparse
import os
import subprocess

class ThreadExecutor(Thread):
    def __init__(self, cmd):
        Thread.__init__(self)
        self.cmd = cmd
    def run(self):
        subprocess.run(self.cmd)

def unzip(zipfolder, unzipfolder):
    """
    unzip all zip files and put into unzip folder with the same name
    """
    if os.path.exists(unzipfolder):
        print('Output folder exists. Remove to continue')
        exit(1)
    for root,dirs,files in os.walk(zipfolder):
        for f in files:
            if f.endswith('.zip'):
                # extract
                tmpfolder=unzipfolder+'/tmp-'+f
                targetfolder = unzipfolder + '/' + f[:-4]
                zipfile = root + '/' + f
                os.makedirs(tmpfolder)
                print ('unzipping ' + zipfile + ' ..')
                subprocess.run('unzip -q ' + zipfile + ' -d ' + tmpfolder, shell=True)
                subprocess.run('mv ' + tmpfolder + '/* ' + targetfolder, shell=True)
                # this folder is empty
                os.rmdir(tmpfolder)

def preprocess(indir, outdir):
    for item in os.listdir(indir):
        benchmark = os.path.join(indir, item)
        outputdir = os.path.join(outdir, item)
        cmd = 'helium --preprocess ' + benchmark + ' -o ' + outputdir
        subprocess.run(cmd, shell=True)
        cmd = 'helium --create-include-dep ' + benchmark\
              + ' -o ' + os.path.join(outputdir, 'include.json')
        subprocess.run(cmd, shell=True)
        cmd = 'helium --create-snippet ' + benchmark\
              + ' --include-dep ' + os.path.join(outputdir, 'include.json')\
              + ' -o ' + os.path.join(outputdir, 'snippets.json')
        subprocess.run(cmd, shell=True)

def create_selection(indir, outdir):
    """
    Create random selection for all cache folder
    """
    for item in os.listdir(indir):
        benchmark=os.path.join(indir, item)
        outputdir=os.path.join(outdir, item)
        if os.path.isdir(benchmark):
            cmd = 'helium --create-sel --sel-num 10 --sel-tok 5 '\
                  + benchmark + ' -o ' + outputdir
            subprocess.run(cmd, shell=True)

if __name__ == '__main__':
    """
    Typical usage:
    ./preprocess.py --preprocess --indir ~/data/test/unzipped/ --outdir ~/data/test/preprocessed
    ./preprocess.py --create-selection --indir ~/data/test/unzipped/ --outdir ~/data/test/selection
    """
    parser = argparse.ArgumentParser(description='Preprocessing Commands')
    parser.add_argument('--unzip', action='store_true')
    parser.add_argument('--preprocess', action='store_true')
    parser.add_argument('--create-selection', action='store_true')
    parser.add_argument('--indir', required=True)
    parser.add_argument('--outdir', required=True)
    args = parser.parse_args();
    if args.unzip:
        unzip(args.indir, args.outdir)
    elif args.preprocess:
        preprocess(args.indir, args.outdir)
    elif args.create_selection:
        create_selection(args.indir, args.outdir)
            
