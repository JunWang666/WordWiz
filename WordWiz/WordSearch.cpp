// WordSearch.cpp
#include "pch.h"
#include "WordSearch.h"
#include "WordSearch.g.cpp" 
#include "WordItem.h"
#include "DictionaryItemInWordDetail.h"

using namespace winrt;
using namespace Windows::Foundation::Collections;

namespace winrt::WordWiz::implementation
{
    Windows::Foundation::Collections::IVector<WordWiz::WordItem> WordSearch::Search(winrt::hstring const& query)
    {
        auto results = winrt::single_threaded_observable_vector<WordWiz::WordItem>();
        if (query.empty())
        {
            return results;
        }
        auto item1 = winrt::make<WordWiz::implementation::WordItem>(query, query + L" (short explanation)");
        results.Append(item1);
        return results;
    }

     Windows::Foundation::Collections::IVector<WordWiz::DictionaryItemInWordDetail> WordSearch::GetAvailableDictionaries(winrt::hstring const& word)
    {
        // 模拟返回带字符串ID的字典列表
        auto dictionaries = winrt::single_threaded_vector<WordWiz::DictionaryItemInWordDetail>();
        
        if (!word.empty())
        {
            dictionaries.Append(WordWiz::DictionaryItemInWordDetail{ winrt::hstring(L"dict_a"), winrt::hstring(L"dict_a"), winrt::hstring(L"词典A") });
            dictionaries.Append(WordWiz::DictionaryItemInWordDetail{ winrt::hstring(L"dict_b"), winrt::hstring(L"dict_b"), winrt::hstring(L"词典B") });
            dictionaries.Append(WordWiz::DictionaryItemInWordDetail{ winrt::hstring(L"oxford_advanced"), winrt::hstring(L"oxford_advanced"), winrt::hstring(L"牛津高阶模拟") });
        }
        return dictionaries;
    }


     winrt::hstring WordSearch::GetDictionaryHtmlContent(winrt::hstring const& word, winrt::hstring const& dictionaryId)
    {
        winrt::hstring html_template_start = L"<html style='background-color:transparent;'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><style>body{background-color:transparent;color:#333;font-family:sans-serif;margin:15px;padding:0;}h1{color:#0078D4;}p{line-height:1.6;}b,strong{font-weight:bold;}i,em{font-style:italic;}ul{padding-left:20px;}</style></head><body style='background-color:transparent;'>";
        winrt::hstring html_template_end = L"</body></html>";

        winrt::hstring content;

        if (dictionaryId == L"dict_a")
        {
            content = L"<h1>" + word + L" - " + dictionaryId + L"</h1><p>这是来自<b>词典A</b>的详细解释（填充内容）。</p><p style='color:green;'>一些特别的笔记。</p>";
        }
        else if (dictionaryId == L"dict_b")
        {
            content =
                L"<head><style type=\"text/css\">\n" // Note the L prefix
                L"@font-face{font-family:'oalecd9';src:url(\"oalecd9.ttf\");font-weight:400;font-style:normal}\n"
                L"body{background-color:#fffefe;font-family:'oalecd9';counter-reset:sn_blk_counter}\n"
                L".cixing_part{counter-reset:sn_blk_counter}\n"
                L".cixing_tiaozhuan_part{display:inline;color:#c70000}\n"
                L".cixing_tiaozhuan_part a:link{text-decoration:none;font-weight:600}\n"
                L".cixing_tiaozhuan_part a{color:#c70000}\n"
                L"h{font-weight:600;color:#323270;font-size:22px}\n"
                L"boxtag{font-size:13px;font-weight:600;border-style:solid;color:#fff;background-color:blue;border-color:blue;border-width:1px;margin-top:2px;padding-left:2px;padding-right:2px;border-radius:10px}\n"
                L"boxtag[type=\"awl\"]{font-size:9px;font-weight:600;color:#fff;border-style:solid;border-width:1px;background-color:#000;border-color:#000;padding-left:1px;padding-right:1px;border-top:0;border-bottom:0}\n"
                L"vp-gs{display:none}\n"
                L"pron-g-blk{display:inline}\n"
                L"top-g{display:block}\n"
                L"pron-g-blk brelabel{padding-left:4px;font-size:14px}\n"
                L"pron-g-blk namelabel{padding-left:4px;font-size:14px}\n"
                L"pos xhtml\\:a{display:table;color:#fff;font-weight:600;padding-left:2px;padding-right:2px;border-style:solid;border-width:1px;border-radius:5px;border-top:0;border-bottom:0;border-color:#c70000;background-color:#c70000}\n"
                L"vpform{color:#9b9b9b;font-style:italic}\n"
                L"vp-g{display:block;padding-left:12px}\n"
                L"sn-blk{display:block}\n"
                L":not(idm-g) sn-gs sn-blk::before{padding-right:4px;counter-increment:sn_blk_counter;content:counter(sn_blk_counter)}\n"
                L":not(id-g) sn-gs sn-blk::before{padding-right:4px;counter-increment:sn_blk_counter;content:counter(sn_blk_counter)}\n"
                L"def{font-weight:600}\n"
                L"xsymb{display:none}\n"
                L"xhtml\\:br{}\n"
                L"x-g-blk{display:block;border-left:3px solid #dbdbdb;margin-left:8px;padding-left:10px}\n"
                L"x-g-blk x::before{content:'•'}\n"
                L"x-g-blk x{font-style:italic;color:#3784dd}\n"
                L"x-g-blk x chn{padding-left:13px;font-style:normal;color:#8d8d8d}\n"
                L"top-g xhtml\\:br{display:none}\n"
                L"cf-blk{font-style:italic;font-weight:600;color:#2b7dca;padding-right:4px}\n"
                L"xr-gs{display:block}\n"
                L"xr-g-blk a:link{text-decoration:none;color:#a52a2a;font-weight:600}\n"
                L"def+x-gs cf-blk{font-style:italic;font-weight:600;color:#2b7dca;display:block}\n"
                L"shcut-blk{margin-top:14px;display:block;border-bottom:1px solid #a0a0a0;padding-bottom:5px}\n"
                L"gram-g{font-weight:600;color:#04b92b}\n"
                L"unbox{margin-top:16px;margin-bottom:16px;display:block;padding-left:5px;padding-right:15px;padding-top:10px;border:1px solid red;border-radius:12px}\n"
                L"unbox title{display:none}\n"
                L"unbox inlinelist{display:inline}\n"
                L"unbox inlinelist und{font-weight:600;color:#03648a}\n"
                L"unbox unsyn{display:block;font-weight:600;color:#1a4781}\n"
                L"unbox x-g-blk{display:block}\n"
                L"unbox x-g-blk x::before{content:'•';padding-right:6px}\n"
                L"unbox h3{color:#36866a;margin-bottom:4px;margin-top:6px}\n"
                L"unbox eb{font-weight:600}\n"
                L"pron-g-blk a:link{text-decoration:none}\n"
                L"audio-gbs-liju,audio-gb-liju,audio-brs-liju,audio-gb{padding-right:4px;color:blue;opacity:.8;display:none}\n"
                L"audio-uss-liju,audio-ams-liju,audio-us-liju,audio-us{padding-right:4px;color:#af0404;opacity:.8;display:none}\n"
                L"a:link{text-decoration:none}\n"
                L"eb{font-weight:600}\n"
                L"idm-gs un{display:block;color:#7c7070}\n"
                L"idm-blk idm{padding-top:12px;display:block;font-weight:600;color:#010102}\n"
                L"idm-g def{font-weight:500}\n"
                L"idm-g sn-blk::before{color:#6f49c7;content:'★'}\n"
                L"pv-g def{font-weight:500}\n"
                L"label-g-blk{color:#797979;font-style:italic}\n"
                L"pv-blk pv{padding-top:12px;display:block;font-weight:600;color:#1881e4}\n"
                L"unbox ul li{list-style-type:square}\n"
                L"unbox x-gs{display:block;margin-left:8px;padding-left:10px}\n"
                L"unbox x-gs chn{}\n"
                L"img{display:block;max-width:100%}\n"
                L".big_pic{display:none;max-width:100%}\n"
                L".switch_ec{display:none}\n"
                L"if-gs-blk{display:inline}\n"
                L"if-gs-blk form{display:inline}\n"
                L"unbox[type=wordfinder] xr-gs{display:inline}\n"
                L"unbox[type=wordfinder]::before{border:1px solid;border-radius:6px;position:relative;top:-24px;font-size:18px;color:#af1919;font-weight:600;content:'WordFinder';background-color:#fff;padding:5px 7px}\n"
                L"unbox[type=colloc]::before{border:1px solid;border-radius:6px;position:relative;top:-24px;left:18px;font-size:18px;color:#af1919;font-weight:600;content:'Collocations 词语搭配';background-color:#fff;padding:5px 7px}\n"
                L"unbox[type=wordfamily]{display:block;float:right}\n"
                L"unbox[type=wordfamily] wfw-g{display:block}\n"
                L"unbox[type=wordfamily] wfw-g wfw-blk{color:#101095;font-weight:600}\n"
                L"unbox[type=wordfamily] wfw-g wfo{font-weight:600}\n"
                L"unbox[type=wordfamily] wfw-g wfp-blk wfp{font-style:italic;color:#971717;font-weight:500}\n"
                L"unbox[type=wordfamily]::before{border:1px solid;border-radius:6px;position:relative;top:-24px;left:18px;font-size:18px;color:#af1919;font-weight:600;content:'WORD FAMILY';background-color:#fff;padding:5px 7px}\n"
                L"unbox[type=grammar]::before{border:1px solid;border-radius:6px;position:relative;top:-24px;left:18px;font-size:18px;color:#af1919;font-weight:600;content:'GRAMMAR 语法';background-color:#fff;padding:5px 7px}\n"
                L"unbox[type=grammar]{margin-top:36px}\n"
                L"unbox[type=grammar] x-gs{padding-left:0;margin-left:0}\n"
                L"unbox ul{margin-top:4px}\n"
                L"use-blk{color:#0b8a0b}\n"
                L"dis-g xr-gs{display:inline}\n"
                L"xr-gs[firstinblock=\"n\"]{display:inline}\n\n"
                L"</style><script src=\"oalecd9.js\" charset=\"utf-8\" type=\"text/javascript\" language=\"javascript\"></script></head>"
                // --------- 分割点 1 ---------
                L"<h-g eid=\"abate_hg_1\"><top-g> <h e_id=\"13cbab9c8f4.ff9\">abate</h><infl e_id=\"13cbab9c8f4.ffb\"></infl><infl e_id=\"13cbab9c8f4.ffc\"></infl><infl e_id=\"13cbab9c8f4.ffd\"></infl><infl e_id=\"13cbab9c8f4.ffe\"></infl>"
                L"<pron e eid=\"abate_prongs_1\" gs href=\"abate_prongs_1\" psg=\"ald8_abate_prongs_1\" resource=\"phonetics\" source=\"ald8\" targeteltid=\"u4cdebea65f7df6b4.65a4283b.1430b37eb0f.1a3b\" wd=\"abate\">"
                L"<pron-g-blk><brelabel><xhtml:a href=\"help:bre\">BrE</xhtml:a></brelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>əˈbeɪt</phon>/</phon-blk><audio eid=\"abate_audio_1\" name=\"abate__gb_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abate__gb_1.spx\"><audio-gb>🔊</audio-gb></a></pron-g-blk>"
                L"<pron-g-blk><xhtml:br></xhtml:br><namelabel><xhtml:a href=\"help:name\">NAmE</xhtml:a></namelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>əˈbeɪt</phon>/</phon-blk><audio eid=\"abate_audio_2\" name=\"abate__us_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abate__us_1.spx\"><audio-us>🔊</audio-us></a></pron-g-blk></pron-gs>"
                L"<pos-g eid=\"abate_posg_1\"> <pos-blk><pos><xhtml:a href=\"helpp:v\">verb</xhtml:a></pos></pos-blk></pos-g>"
                L"<res-g eid=\"abate_resg_1\"><vp-gs eid=\"abate_vpgs_1\">"
                L"<vpform><xhtml:a href=\"helpil:root\">present simple - I / you / we / they</xhtml:a> </vpform><vp-g form=\"root\"><vp eid=\"abate_vp_1\">abate</vp><pron e eid=\"abate_prongs_1\" gs href=\"abate_prongs_1\" psg=\"ald8_abate_prongs_1\" resource=\"phonetics\" source=\"ald8\" targeteltid=\"u4cdebea65f7df6b4.65a4283b.1430b37eb0f.1a3b\" wd=\"abate\"><pron-g-blk> <brelabel><xhtml:a href=\"help:bre\">BrE</xhtml:a></brelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>əˈbeɪt</phon>/</phon-blk><audio eid=\"abate_audio_1\" name=\"abate__gb_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abate__gb_1.spx\"><audio-gb>🔊</audio-gb></a></pron-g-blk><pron-g-blk> <namelabel><xhtml:a href=\"help:name\">NAmE</xhtml:a></namelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>əˈbeɪt</phon>/</phon-blk><audio eid=\"abate_audio_2\" name=\"abate__us_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abate__us_1.spx\"><audio-us>🔊</audio-us></a></pron-g-blk></pron-gs></vp-g>"
                L"<vpform><xhtml:a href=\"helpil:thirdps\">present simple - he / she / it</xhtml:a> </vpform><vp-g form=\"thirdps\"><vp eid=\"abate_vp_2\">abates</vp><pron e eid=\"abate_prongs_3\" gs href=\"abate_prongs_3\" psg=\"ald8_abate_prongs_3\" resource=\"phonetics\" source=\"ald8\" targeteltid=\"u4cdebea65f7df6b4.65a4283b.1430b37eb0f.1a5f\" wd=\"abates\"><pron-g-blk> <brelabel><xhtml:a href=\"help:bre\">BrE</xhtml:a></brelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>əˈbeɪts</phon>/</phon-blk><audio eid=\"abates_audio_1\" name=\"abates__gb_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abates__gb_1.spx\"><audio-gb>🔊</audio-gb></a></pron-g-blk><pron-g-blk> <namelabel><xhtml:a href=\"help:name\">NAmE</xhtml:a></namelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>əˈbeɪts</phon>/</phon-blk><audio eid=\"abates_audio_2\" name=\"abates__us_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abates__us_1.spx\"><audio-us>🔊</audio-us></a></pron-g-blk></pron-gs></vp-g>"
                // --------- 分割点 2 ---------
                L"<vpform><xhtml:a href=\"helpil:past\">past simple</xhtml:a> </vpform><vp-g form=\"past\"><vp eid=\"abate_vp_3\">abated</vp><pron-gs eid=\"abate_prongs_5\" psg=\"ald8_abate_prongs_4\" source=\"gl2018\" wd=\"abated\"><pron-g-blk> <brelabel><xhtml:a href=\"help:bre\">BrE</xhtml:a></brelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>əˈbeɪtɪd</phon>/</phon-blk><audio eid=\"abated_audio_1\" name=\"abated__gb_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abated__gb_1.spx\"><audio-gb>🔊</audio-gb></a></pron-g-blk><pron-g-blk> <namelabel><xhtml:a href=\"help:name\">NAmE</xhtml:a></namelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>əˈbeɪtɪd</phon>/</phon-blk><audio eid=\"abated_audio_2\" name=\"abated__us_2\"></audio></xhtml:a></pron-g> <a href=\"snd://abated__us_2.spx\"><audio-us>🔊</audio-us></a></pron-g-blk></pron-gs></vp-g>"
                L"<vpform><xhtml:a href=\"helpil:pastpart\">past participle</xhtml:a> </vpform><vp-g form=\"pastpart\"><vp eid=\"abate_vp_4\">abated</vp><pron-gs eid=\"abate_prongs_5\" psg=\"ald8_abate_prongs_4\" source=\"gl2018\" wd=\"abated\"><pron-g-blk> <brelabel><xhtml:a href=\"help:bre\">BrE</xhtml:a></brelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>əˈbeɪtɪd</phon>/</phon-blk><audio eid=\"abated_audio_1\" name=\"abated__gb_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abated__gb_1.spx\"><audio-gb>🔊</audio-gb></a></pron-g-blk><pron-g-blk> <namelabel><xhtml:a href=\"help:name\">NAmE</xhtml:a></namelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>əˈbeɪtɪd</phon>/</phon-blk><audio eid=\"abated_audio_2\" name=\"abated__us_2\"></audio></xhtml:a></pron-g> <a href=\"snd://abated__us_2.spx\"><audio-us>🔊</audio-us></a></pron-g-blk></pron-gs></vp-g>"
                L"<vpform><xhtml:a href=\"helpil:prespart\"> -ing form</xhtml:a> </vpform><vp-g form=\"prespart\"><vp eid=\"abate_vp_5\">abating</vp><pron e eid=\"abate_prongs_6\" gs href=\"abate_prongs_6\" psg=\"ald8_abate_prongs_5\" resource=\"phonetics\" source=\"ald8\" targeteltid=\"u4cdebea65f7df6b4.65a4283b.1430b37eb0f.1a6b\" wd=\"abating\"><pron-g-blk> <brelabel><xhtml:a href=\"help:bre\">BrE</xhtml:a></brelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>əˈbeɪtɪŋ</phon>/</phon-blk><audio eid=\"abating_audio_1\" name=\"abating__gb_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abating__gb_1.spx\"><audio-gb>🔊</audio-gb></a></pron-g-blk><pron-g-blk> <namelabel><xhtml:a href=\"help:name\">NAmE</xhtml:a></namelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>əˈbeɪtɪŋ</phon>/</phon-blk><audio eid=\"abating_audio_2\" name=\"abating__us_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abating__us_1.spx\"><audio-us>🔊</audio-us></a></pron-g-blk></pron-gs></vp-g></vp-gs></res-g>"
                L"<gram-g eid=\"abate_gramg_1\"><gram-blk> [<gram><xhtml:a href=\"helpgr:i\">intransitive</xhtml:a></gram></gram-blk><gram-blk>, <gram><xhtml:a href=\"helpgr:t\">transitive</xhtml:a></gram>] </gram-blk></gram-g><label-g-blk> (<label-g><reg-blk><reg e_id=\"13cbab9c8f4.101f\" reg=\"fml\"><xhtml:a href=\"helpr:fml\">formal</xhtml:a></reg></reg-blk></label-g>) </label-g-blk></top-g>"
                // --------- 分割点 3 ---------
                L"<sn-gs eid=\"abate_sngs_1\"><sn-blk-nolist><sn-g eid=\"abate_sng_1\"><def neh=\"n\" noblk=\"y\"><xhtml:a href=\"d:to\">to</xhtml:a> <xhtml:a href=\"d:become\">become</xhtml:a> <xhtml:a href=\"d:less\">less</xhtml:a> <xhtml:a href=\"d:strong\">strong</xhtml:a>; <xhtml:a href=\"d:to\">to</xhtml:a> <xhtml:a href=\"d:make\">make</xhtml:a> <xhtml:a href=\"help:sth\">sth</xhtml:a> <xhtml:a href=\"d:less\">less</xhtml:a> <xhtml:a href=\"d:strong\">strong</xhtml:a><chnsep> </chnsep><chn><fthzmark></fthzmark>（使）减弱，减退，减轻，减少</chn></def>"
                L"<x-gs eid=\"abate_xgs_1\"><x-g-blk e_id=\"13cbab9c8f4.1024\" eid=\"abate_xg_1\"><xsymb><xhtml:a href=\"addexample:13cbab9c8f4.1024\">◆</xhtml:a></xsymb><rx e eid=\"abate_rxg_1\" g href=\"abate_rxg_1\" resource=\"recx\" source=\"ald9\" targeteltid=\"13cba85ad41.6fdc\"><x e_id=\"u4cdebea65f7df6b4.1f34c209.141c0d51fe7.-2331\" eid=\"abate_x_1\" status=\"6\" tranid=\"2\" wd=\"The storm showed no signs of abating.\"><xhtml:a href=\"x:The\">The</xhtml:a> <xhtml:a href=\"x:storm\">storm</xhtml:a> <xhtml:a href=\"x:showed\">showed</xhtml:a> <xhtml:a href=\"x:no\">no</xhtml:a> <xhtml:a href=\"x:signs\">signs</xhtml:a> <xhtml:a href=\"x:of\">of</xhtml:a> <xhtml:a href=\"x:abating\">abating</xhtml:a>.  <xhtml:br></xhtml:br><chn><fthzmark></fthzmark>暴风雨没有减弱的迹象。</chn></x><a href=\"snd://_abate__gbs_1.spx\"><audio-gbs-liju>🔊</audio-gbs-liju></a><a href=\"snd://_abate__uss_1.spx\"><audio-uss-liju>🔊</audio-uss-liju></a></rx-g></x-g-blk>"
                L"<x-g-blk e_id=\"13cbab9c8f4.1028\" eid=\"abate_xg_3\"><xsymb><xhtml:a href=\"addexample:13cbab9c8f4.1028\">◆</xhtml:a></xsymb><cf-blk><cf><exp>~</exp> <xhtml:a href=\"help:sth\">sth</xhtml:a></cf> </cf-blk><rx e eid=\"abate_rxg_2\" g href=\"abate_rxg_2\" resource=\"recx\" source=\"ald9\" targeteltid=\"13cba85ad41.6fe1\"><x e_id=\"u4cdebea65f7df6b4.1f34c209.141c0d51fe7.-232d\" eid=\"abate_x_3\" status=\"6\" tranid=\"3\" wd=\"Steps are to be taken to abate pollution.\"><xhtml:a href=\"x:Steps\">Steps</xhtml:a> <xhtml:a href=\"x:are\">are</xhtml:a> <xhtml:a href=\"x:to\">to</xhtml:a> <xhtml:a href=\"x:be\">be</xhtml:a> <xhtml:a href=\"x:taken\">taken</xhtml:a> <xhtml:a href=\"x:to\">to</xhtml:a> <xhtml:a href=\"x:abate\">abate</xhtml:a> <xhtml:a href=\"x:pollution\">pollution</xhtml:a>.  <xhtml:br></xhtml:br><chn><fthzmark></fthzmark>将会采取措施减少污染。</chn></x><a href=\"snd://_abate__gbs_2.spx\"><audio-gbs-liju>🔊</audio-gbs-liju></a><a href=\"snd://_abate__uss_2.spx\"><audio-uss-liju>🔊</audio-uss-liju></a></rx-g></x-g-blk></x-gs></sn-g></sn-blk-nolist></sn-gs>"
                L"<dr-gs eid=\"abate_drgs_1\"><dr-g-blk><dr-g e_id=\"13cbab9c8f4.102d\" eid=\"abate_drg_1\"><drtri><xhtml:a href=\"adddr:13cbab9c8f4.102d\">▸</xhtml:a></drtri> <top-g><dr-blk><dr e_id=\"13cbab9c8f4.102f\" eid=\"abate_dr_1\">abate·ment</dr> </dr-blk><pron e eid=\"abate_prongs_7\" gs href=\"abate_prongs_7\" psg=\"ald8_abate_prongs_8\" resource=\"phonetics\" source=\"ald8\" targeteltid=\"u4cdebea65f7df6b4.65a4283b.1430b37eb0f.1a53\" wd=\"abatement\"><pron-g-blk><brelabel><xhtml:a href=\"help:bre\">BrE</xhtml:a></brelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>əˈbeɪtmənt</phon>/</phon-blk><audio eid=\"abatement_audio_1\" name=\"abatement__gb_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abatement__gb_1.spx\"><audio-gb>🔊</audio-gb></a></pron-g-blk><pron-g-blk><xhtml:br></xhtml:br><namelabel><xhtml:a href=\"help:name\">NAmE</xhtml:a></namelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>əˈbeɪtmənt</phon>/</phon-blk><audio eid=\"abatement_audio_2\" name=\"abatement__us_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abatement__us_1.spx\"><audio-us>🔊</audio-us></a></pron-g-blk></pron-gs><pos-g eid=\"abate_posg_2\"> <pos-blk><pos><xhtml:a href=\"helpp:n\">noun</xhtml:a></pos></pos-blk></pos-g></top-g><sn-gs eid=\"abate_sngs_2\"><sn-blk-nolist><sn-g eid=\"abate_sng_2\"><gram-g eid=\"abate_gramg_2\"><gram-blk> [<gram><xhtml:a href=\"helpgr:u\">uncountable</xhtml:a></gram>] </gram-blk></gram-g></sn-g></sn-blk-nolist></sn-gs></dr-g></dr-g-blk></dr-gs></h-g>";
        }
        else if (dictionaryId == L"oxford_advanced")
        {
            return L"<html style='background-color:transparent;'><head><meta charset='UTF-8'><style>body { background-color:transparent; font-family: sans-serif; margin: 20px; color: #333; } .word { color: #c00; font-size: 2em; } .phonetic { color: #555; } .definition { margin-top: 10px; border-left: 3px solid #007acc; padding-left: 10px; }</style></head>"
                L"<body style='background-color:transparent;'><div class='word'>" + word + L"</div><div class='phonetic'>[ˈwɜːd]</div>"
                L"<div class='definition'><b>(noun)</b> a single distinct meaningful element of speech or writing, used with others (or sometimes alone) to form a sentence and typically shown with a space on either side when written or printed.</div>"
                L"<div class='definition'><b>(verb)</b> express (something) in particular words.</div>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"</body></html>";
        }
        else
        {
            content = L"<p>未找到字典 " + dictionaryId + L" 中关于 " + word + L" 的内容。</p>";
        }

        return html_template_start + content + html_template_end;
    }

}