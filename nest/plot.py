import numpy as np
import matplotlib.pyplot as plt
import math
import ROOT as r

r.gROOT.SetStyle('Plain')
r.gStyle.SetOptStat('000000')
r.gStyle.SetLegendBorderSize(0)


def format_histo(histo, color):
	histo.GetXaxis().SetTitle("S1")
	histo.GetYaxis().SetTitle("log(S2/S1)")
	histo.GetYaxis().SetTitleOffset(1.25)
	
	histo.SetTitle("")
	histo.SetMarkerStyle(20);
	histo.SetMarkerSize(0.8);
	histo.SetMarkerColorAlpha(color, 0.5);


def read_nest_files(file, histo ):
	nlines=0
	num_lines = sum(1 for line in open(file))
	print num_lines
	for line in open(file):
		nlines=nlines+1
		elems = line.split()
		if nlines <5:
			continue
		s1=float(elems[9])
		s2=float(elems[13])
		if nlines >4 and nlines <10:
			print file+' '+str(s1)+' '+str(s2)+' '+str(math.log(s2,10))
		if s1<0 or s2<0:
			continue
		logs2s1.append(math.log(s2/s1))
		s1list.append(s1)
		s2list.append(s2)
		histo.Fill(s1, math.log(s2/s1,10))
	print 'found '+str(nlines)+' data points in'+' '+file

histoNR = r.TH2F('NR','NR', 2000, 0., 700, 500, 0, 4 )
histoER = r.TH2F('ER','ER', 2000, 0., 700, 500, 0, 4 )
s1list=[]
s2list=[]
logs2s1=[]
read_nest_files('testNR2.dat', histoNR)
read_nest_files('testER2.dat', histoER)

#plt.scatter(s1, logs2s1)
#plt.show()
#plt.savefig('foo.png')


c1 = r.TCanvas('canv', 'canv',1440, 1080)
format_histo(histoNR, r.kRed)
format_histo(histoER, r.kBlue)
histoNR.Draw()
histoER.Draw("same")
legend=r.TLegend(0.75,0.75,0.89,0.89)
legend.SetFillStyle(0)
legend.AddEntry(histoNR, histoNR.GetName(), "p")
legend.AddEntry(histoER, histoER.GetName(), "p")
legend.Draw()
c1.SaveAs('s1_s2_plot.png')



c2 = r.TCanvas('canv2', 'canv2',2000, 1080)
center_pad = r.TPad('center_pad', 'center_pad',0.50,0.0,1.0,1.0);
center_pad.Draw();
right_pad = r.TPad('right_pad', 'right_pad',0.0,0.0,0.5,1.0);
right_pad.Draw();

proj_NR_Y = histoNR.ProjectionY();
proj_ER_Y = histoER.ProjectionY();

center_pad.cd()
histoNR.Draw()
histoER.Draw('same')
right_pad.cd()
proj_NR_Y.SetFillColorAlpha(r.kRed, 0.6)
proj_ER_Y.SetFillColorAlpha(r.kBlue, 0.7)
proj_ER_Y.Draw('hbar')
proj_NR_Y.Draw('hbarsame')

legend=r.TLegend(0.75,0.75,0.89,0.89)
legend.SetFillStyle(0)
legend.AddEntry(histoNR, histoNR.GetName(), "p")
legend.AddEntry(histoER, histoER.GetName(), "p")
legend.Draw()
c2.SaveAs('s1_s2_plot_w_projection.png')
