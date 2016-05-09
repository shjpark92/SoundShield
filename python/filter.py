
from pyfilterbank import FractionalOctaveFilterbank
from pyfilterbank import octbank

ofb = FractionalOctaveFilterbank(sample_rate = 44100, order=30, nth_oct=1, norm_freq = 960,start_band = -5,end_band=4,edge_correction_percent=0.2,filterfun='py')
