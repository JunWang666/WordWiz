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
            content = L"<head><link rel=\"stylesheet\" type=\"text/css\" href=\"oalecd9.css\" /><script src=\"oalecd9.js\" charset=\"utf-8\" type=\"text/javascript\" language=\"javascript\"></script></head><h-g eid=\"abacus_hg_1\"><top-g> <h e_id=\"13cbab9c8f4.eac\">aba·cus</h><infl e_id=\"13cbab9c8f4.eae\"></infl><infl e_id=\"13cbab9c8f4.eaf\"></infl><infl e_id=\"13cbab9c8f4.eb0\"></infl><pron e eid=\"abacus_prongs_1\" gs href=\"abacus_prongs_1\" psg=\"ald8_abacus_prongs_1\" resource=\"phonetics\" source=\"ald8\" targeteltid=\"u4cdebea65f7df6b4.65a4283b.1430b37eb0f.1983\" wd=\"abacus\"><pron-g-blk><brelabel><xhtml:a href=\"help:bre\">BrE</xhtml:a></brelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>ˈæbəkəs</phon>/</phon-blk><audio eid=\"abacus_audio_1\" name=\"abacus__gb_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abacus__gb_1.spx\"><audio-gb>🔊</audio-gb></a></pron-g-blk><pron-g-blk><xhtml:br></xhtml:br><namelabel><xhtml:a href=\"help:name\">NAmE</xhtml:a></namelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>ˈæbəkəs</phon>/</phon-blk><audio eid=\"abacus_audio_2\" name=\"abacus__us_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abacus__us_1.spx\"><audio-us>🔊</audio-us></a></pron-g-blk></pron-gs><pos-g eid=\"abacus_posg_1\"> <pos-blk><pos><xhtml:a href=\"helpp:n\">noun</xhtml:a></pos></pos-blk></pos-g><res-g eid=\"abacus_resg_1\"></res-g><if-gs-blk> (<if-gs><if-g-blk><form><xhtml:a href=\"helpil:pl\">plural</xhtml:a> </form><if-g><if-blk><if e_id=\"13cbab9c8f4.eba\">aba·cuses</if></if-blk><pron e eid=\"abacus_prongs_2\" gs href=\"abacus_prongs_2\" psg=\"ald8_abacus_prongs_2\" resource=\"phonetics\" source=\"ald8\" targeteltid=\"u4cdebea65f7df6b4.65a4283b.1430b37eb0f.198f\" wd=\"abacuses\"><pron-g-blk> <brelabel><xhtml:a href=\"help:bre\">BrE</xhtml:a></brelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>ˈæbə<ptl eid=\"abacuses_ptl_1\" psg=\"ald8book\">kəsɪz</ptl></phon>/</phon-blk><audio eid=\"abacuses_audio_1\" name=\"abacuses__gb_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abacuses__gb_1.spx\"><audio-gb>🔊</audio-gb></a> </pron-g-blk><pron-g-blk> <namelabel><xhtml:a href=\"help:name\">NAmE</xhtml:a></namelabel> <pron-g><xhtml:a href=\"help:phonetics\"><phon-blk>/<phon>ˈæbə<ptl eid=\"abacuses_ptl_2\" psg=\"ald8book\">kəsɪz</ptl></phon>/</phon-blk><audio eid=\"abacuses_audio_2\" name=\"abacuses__us_1\"></audio></xhtml:a></pron-g> <a href=\"snd://abacuses__us_1.spx\"><audio-us>🔊</audio-us></a> </pron-g-blk></pron-gs></if-g></if-g-blk></if-gs>) </if-gs-blk></top-g><sn-gs eid=\"abacus_sngs_1\"><sn-blk-nolist><sn-g eid=\"abacus_sng_1\"><def neh=\"n\" noblk=\"y\"><xhtml:a href=\"d:a\">a</xhtml:a> <xhtml:a href=\"d:frame\">frame</xhtml:a> <xhtml:a href=\"d:with\">with</xhtml:a> <xhtml:a href=\"d:small\">small</xhtml:a> <xhtml:a href=\"d:balls\">balls</xhtml:a> <xhtml:a href=\"d:which\">which</xhtml:a> <xhtml:a href=\"d:slide\">slide</xhtml:a> <xhtml:a href=\"d:along\">along</xhtml:a> <xhtml:a href=\"d:wires\">wires</xhtml:a>. <xhtml:a href=\"d:It\">It</xhtml:a> <xhtml:a href=\"d:is\">is</xhtml:a> <xhtml:a href=\"d:used\">used</xhtml:a> <xhtml:a href=\"d:as\">as</xhtml:a> <xhtml:a href=\"d:a\">a</xhtml:a> <xhtml:a href=\"d:tool\">tool</xhtml:a> <xhtml:a href=\"d:or\">or</xhtml:a> <xhtml:a href=\"d:toy\">toy</xhtml:a> <xhtml:a href=\"d:for\">for</xhtml:a> <xhtml:a href=\"d:counting\">counting</xhtml:a>.<chnsep> </chnsep><chn><fthzmark></fthzmark>算盘</chn></def></sn-g></sn-blk-nolist></sn-gs></h-g>";
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