#ifndef GUARD_BERRY_PLOTS_H
#define GUARD_BERRY_PLOTS_H

bool32 IsBerryPlotHidden(const struct ObjectEventTemplate *template);
bool32 RestoreCollectedBerryPlot(struct ObjectEventTemplate *template);
void RestoreBerryPlotAfterPickup(void);

#endif
