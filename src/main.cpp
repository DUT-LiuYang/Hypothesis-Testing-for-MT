/**
SMT bleu sign test
written by Zhongjun He, ICT, CAS
zjhe@ict.ac.cn
2008-01-07
*/

//�ȼ���baseline��bleuֵ��Ȼ��˳���ҳ��Ա�ϵͳ����baseline��ͬ���У������滻���������
//�����µ�bleuֵ����baseline����ô˵���Ա�ϵͳ����仰����Ǻõģ��������baseline��˵���Ա�ϵͳ
//��仰�ķ����ǲ��õ�

#include "Eval.h"
#include "Utils.h"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <math.h>
using namespace std;

void Help()
{
	cout << "Sing Test for SMT systems \n"
		<< "written by He Zhongjun, ICT, CAS\n"
		<< "please input:\n"
		<< "-b : the baseline output\n"
		<< "-t : the system output\n"
		<< "-s : the source set\n"
		<< "-r : the reference\n"
		<< "-c: case sensitive\n";
     exit(1);
}

bool caseSensitive = false;

void main(int argc, char* argv[])
{
     
	if(argc < 5)
	{
		Help();
	}

	string baseName("iwslt05_base.flat"), sysName("iwslt05_con.flat");
	string srcName("iwslt05.tst-flat"), refName("iwslt05.tst-ref-g");

	int i=0;
	for(i=1; i<argc; i++)
	{
		string t(argv[i]);

		if(t == "-b")
		{
			baseName = argv[++i];
		}
		else if (t == "-t")
		{
			sysName = argv[++i];
		}
		else if(t == "-s")
		{
			srcName = argv[++i];
		}
		else if(t == "-r")
		{
			refName = argv[++i];
		}
		else if (t == "-c")
		{
			caseSensitive = true;
		}
		else 
		{
			cout << "parameter error";
			Help();
		}
	}

	

	vector<ngram_struct> baseNgram, sysNgram; //ngram information
	vector<string> baseTrans, sysTrans, source; // translation

	cout << "read source text..." << endl;
	int SentNumber = ReadSentence(srcName, source);
    cout << SentNumber << endl;
	vector<vector<string> > reference(SentNumber);
	cout << "read reference..." << endl;
	if(!ReadReference(refName,reference,SentNumber))
		return;
/*	ofstream logs("log.txt");
	for(i=0; i<source.size(); i++)
	{
		logs << source[i] << endl;
		for(int j=0; j<reference[i].size(); j++)
		{
			logs << reference[i][j] << endl;
		}
		logs << endl;
	}*/

	cout << "read baseline..." << endl;
    ReadSentence(baseName, baseTrans);


	cout << "read system translation..." << endl;
	ReadSentence(sysName, sysTrans);



	//����Ngram��Ϣ

	ngram_struct baseAll, sysAll;
	for(i=0; i<baseTrans.size(); i++)
	{
		ngram_struct ns = Get_ngram(reference[i],baseTrans[i]);
		baseNgram.push_back(ns);
       
		baseAll = baseAll + ns;
	}
 //   cout << "baseAll " << baseAll << endl;
	for(i=0; i<sysTrans.size(); i++)
	{
		ngram_struct ns = Get_ngram(reference[i], sysTrans[i]);
		sysNgram.push_back(ns);

		sysAll = sysAll + ns;
	}
 //   cout << "sysAll " << sysAll << endl;
   
	//����bleuֵ
    double bleuBase = BLEU(baseAll);
	double bleuSys = BLEU(sysAll);
	cout << "the baseline ngram: " << baseAll << endl;
	cout << "the baseline BLEU: " << bleuBase << endl;
	cout << "the system ngram: " << sysAll << endl;
	cout << "the system BLEU: " << bleuSys << endl;

   //  exit(1);
	ofstream goodOut("good.xml"), badOut("bad.xml"), equalOut("equal.xml");
	int good=0, bad=0, equal=0, all=0;

	double goodRate=0.0, badRate=0.0;
	for(i=0; i<baseTrans.size(); i++)
	{
		ngram_struct change = baseAll;

		if(baseTrans[i] != sysTrans[i])
		{
			all++;

			change = change - baseNgram[i]; //��ȥbaseline��Ӧ��ngram
			change = change + sysNgram[i]; //������Ӧ��system  ngram
			double newBleu = BLEU(change);

			if(newBleu > bleuBase) //�����
			{
				good++;
				goodOut << "<newBleu: " << newBleu << " baseBleu: " << bleuBase << ">" << endl;
				Print(goodOut,source[i] , reference[i], baseTrans[i] ,sysTrans[i]);
			    goodOut << endl;
				goodRate += newBleu - bleuBase;
			}
			else if (newBleu < bleuBase) //�仵��
			{
				bad++;
				badOut << "<newBleu: " << newBleu << " baseBleu: " << bleuBase << ">" << endl;
                Print(badOut,source[i] , reference[i], baseTrans[i] ,sysTrans[i]);
				badOut << endl;
				badRate += bleuBase - newBleu;
			}
		    else //���
			{
				equal++;
				equalOut << "<newBleu: " << newBleu << " baseBleu: " << bleuBase << ">" << endl;
				Print(equalOut,source[i] , reference[i], baseTrans[i] ,sysTrans[i]);
				equalOut << endl;
			}
		}
	}

	cout << "total sentences: " << SentNumber << endl;
	cout << "different sentences: " << all  << endl;
	cout << "\tgood: " << good << " " << (double)good*100/(double)all << "%" << endl;
	cout << "\tbad: " << bad << " " << (double)bad*100/(double)all << "%" << endl;
	cout << "\tequal: " << equal << " " << (double)equal*100/(double)all << "%" << endl;

	cout << "good total increase: " << goodRate << endl;
	cout << "bad total decrease: " << badRate << endl;


    //compute sing test
	double N = good + bad;
	double z = fabs(good - N/2) / sqrt(N/4);
	cout << " Z = " << z << endl;
  
	if(z > 2.576)
	{
		cout << z << "> 2.576 statistical level: 0.01" << endl;
	}
	else if(z > 1.96)
	{
		cout << z << "> 1.96 statistical level: 0.05" << endl;
	}
	else
	{
		cout << "not statistically significant!" << endl;
	}

}