
#define NSTAGES 2

typedef struct 
{
    int   t[NSTAGES]; // Timings
    float a[NSTAGES]; // Amplitude levels
    float f[NSTAGES]; // Coefficients
    int   stage;
    int   depated;
    float feed;

} envelope;

void init_envelope(envelope* o)
{
    o->stage = 0;
    o->depated = 0;
    o->feed = 0.0f;
    int ai = 0;
    for(int i = 0; i < NSTAGES; i++)
    {
        o->f[i] = (o->a[i] - ai)/o->t[i];
        ai = o->a[i];
    }
}

float process_envelope(envelope* o)
{
    if(o->depated >= o->t[o->stage]) 
    {
        o->depated = 0;
        o->stage++;
        if(o->stage == NSTAGES) o->stage = 0;
    }
    o->feed += o->f[o->stage];
    o->depated++;
    return o->feed;
}