#ifndef CAPTURE_SOMAGIC_ANALYZER_H_
#define CAPTURE_SOMAGIC_ANALYZER_H_

#include "base/base.h"
#include "capture/analyzer.h"
#include "gui/bounding_box.h"  // TODO(mayah): Consider removing this

struct Box;
struct HSV;
struct RGB;

struct BoxAnalyzeResult {
    BoxAnalyzeResult(RealColor rc, bool vanishing)
    {
        this->realColor = rc;
        this->vanishing = vanishing;
    }

    RealColor realColor;
    bool vanishing;
};

class SomagicAnalyzer : public Analyzer {
public:
    SomagicAnalyzer();
    virtual ~SomagicAnalyzer();

    BoxAnalyzeResult analyzeBox(const SDL_Surface* surface, const Box& b, bool showsColor = false) const;

    // Draw each pixel of |surface| with RealColor. This is helpful for image analyzing test.
    void drawWithAnalysisResult(SDL_Surface*);

private:
    virtual CaptureGameState detectGameState(const SDL_Surface*) OVERRIDE;
    virtual std::unique_ptr<DetectedField> detectField(int pi, const SDL_Surface*) OVERRIDE;

    bool isLevelSelect(const SDL_Surface*);
    bool isGameFinished(const SDL_Surface*);

    void drawBoxWithAnalysisResult(SDL_Surface*, const Box&);
};

#endif
