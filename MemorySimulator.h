#ifndef MEMORYSIMULATOR_H
#define MEMORYSIMULATOR_H

#include <vector>
#include <deque>
#include <algorithm>
#include <limits>

// ------------------ Políticas y Config ------------------
enum class ReplacementPolicy { FIFO, LRU, OPT };

struct MemConfig {
    int frames = 3;
    ReplacementPolicy policy = ReplacementPolicy::FIFO;
    int tlbSize = 0;         // 0 = TLB desactivada
    int costTLBHit = 1;
    int costMemAccess = 10;
    int costPageFault = 200;
};

// ------------------ Paso y Resultados -------------------
struct Step {
    int index = 0;                  // Paso 1..N
    int page  = -1;                 // Referencia
    std::vector<int> frames;        // Estado de marcos (-1 libre)
    bool tlbHit = false;
    bool pageFault = false;
    int victimFrame = -1;           // marco expulsado (si hubo)
    int victimPage  = -1;           // página expulsada (si hubo)
    int timeCost = 0;               // costo del acceso (simulado)
};

struct MemResult {
    std::vector<Step> steps;
    int totalFaults = 0;
    int tlbHits = 0;
    int tlbMiss = 0;
    long long totalTime = 0;

    double faultRate() const { int n=(int)steps.size(); return n? double(totalFaults)/n : 0.0; }
    double tlbHitRate() const { int n=tlbHits+tlbMiss;  return n? double(tlbHits)/n : 0.0; }
    double avgAccessTime() const { int n=(int)steps.size(); return n? double(totalTime)/n : 0.0; }
};

// ------------------ TLB (LRU por edad) ------------------
struct TlbEntry { int page=-1, frame=-1, age=0; };

class TLB {
public:
    explicit TLB(int size=0): n(size), v(size) {}
    void reset(int size){ n=size; v.assign(size, {}); }
    int probe(int p){
        if(n<=0) return -1;
        for(auto& e:v) if(e.page==p){ e.age=0; return e.frame; }
        return -1;
    }
    void insert(int p,int f){
        if(n<=0) return;
        for(auto& e:v) if(e.page==-1){ e={p,f,0}; return; }
        auto it = std::max_element(v.begin(), v.end(),
                                   [](const TlbEntry&a,const TlbEntry&b){return a.age<b.age;});
        *it = TlbEntry{p,f,0};
    }
    void invalidatePage(int p){ if(n<=0) return; for(auto& e:v) if(e.page==p) e={-1,-1,0}; }
    void tick(){ for(auto& e:v) if(e.page!=-1) e.age++; }
private:
    int n; std::vector<TlbEntry> v;
};

// --------------- Simulador ---------------
class MemorySimulator {
public:
    // ✅ Ahora simulateAll ya NO empuja pasos (step() lo hace)
    MemResult simulateAll(const std::vector<int>& pages_, const MemConfig& cfg_){
        begin(pages_, cfg_);
        Step s;
        while(step(s)) { /* step ya guarda en result.steps */ }
        return result;
    }

    void begin(const std::vector<int>& pages_, const MemConfig& cfg_){
        pages = pages_; cfg = cfg_; t = 0;
        frames.assign(cfg.frames, -1);
        fifoQ.clear();
        lruTime.assign(cfg.frames, -1);
        tlb.reset(cfg.tlbSize);
        result = {};
    }

    // ✅ step() ahora también agrega el paso a result.steps
    bool step(Step& out){
        if(t >= (int)pages.size()) return false;

        int p = pages[t];
        Step s; s.index=t+1; s.page=p; s.frames=frames;

        tlb.tick();
        int f = tlb.probe(p);
        if(f != -1){
            s.tlbHit = true; result.tlbHits++; s.timeCost += cfg.costTLBHit;
            onHitLRU(f);
        } else {
            result.tlbMiss++;
            int in = indexOf(frames, p);
            if(in != -1){
                s.timeCost += cfg.costMemAccess;
                f = in; onHitLRU(f);
            } else {
                s.pageFault = true; result.totalFaults++;
                s.timeCost += cfg.costMemAccess + cfg.costPageFault;

                int victim = findFreeFrame();
                if(victim == -1){
                    switch(cfg.policy){
                    case ReplacementPolicy::FIFO: victim = victimFIFO(); break;
                    case ReplacementPolicy::LRU:  victim = victimLRU();  break;
                    case ReplacementPolicy::OPT:  victim = victimOPT();  break;
                    }
                    s.victimFrame = victim;
                    s.victimPage  = frames[victim];
                    tlb.invalidatePage(frames[victim]);
                } else {
                    fifoQ.push_back(victim);
                }

                frames[victim] = p;
                f = victim;
                onHitLRU(f);
            }
            tlb.insert(p,f);
        }

        s.frames = frames;
        result.totalTime += s.timeCost;

        // 🔹 Guardar el paso para métricas en vivo (PFR / AAT)
        result.steps.push_back(s);

        out = s;
        t++;
        return true;
    }

    const MemResult& partial() const { return result; }

private:
    // -------- Estado --------
    std::vector<int> pages; MemConfig cfg{}; int t=0;
    std::vector<int> frames;
    std::deque<int>  fifoQ;
    std::vector<int> lruTime;
    TLB tlb{0};
    MemResult result;

    // -------- Utilidades --------
    static int indexOf(const std::vector<int>& v,int x){ for(int i=0;i<(int)v.size();++i) if(v[i]==x) return i; return -1; }
    int findFreeFrame(){ for(int i=0;i<(int)frames.size();++i) if(frames[i]==-1) return i; return -1; }

    int victimFIFO(){ int vf = fifoQ.front(); fifoQ.pop_front(); fifoQ.push_back(vf); return vf; }
    int victimLRU(){ int best=-1, bestTs=std::numeric_limits<int>::max();
        for(int f=0; f<(int)frames.size(); ++f){ if(lruTime[f] < bestTs){ bestTs=lruTime[f]; best=f; } }
        return best; }
    int victimOPT(){
        int victim=0, farthest=-1;
        for(int f=0; f<(int)frames.size(); ++f){
            int page = frames[f], next=-1;
            for(int j=t+1; j<(int)pages.size(); ++j){ if(pages[j]==page){ next=j; break; } }
            if(next==-1) return f;
            if(next>farthest){ farthest=next; victim=f; }
        }
        return victim;
    }
    void onHitLRU(int frame){ if(cfg.policy!=ReplacementPolicy::FIFO) lruTime[frame] = t; }
};

#endif // MEMORYSIMULATOR_H
