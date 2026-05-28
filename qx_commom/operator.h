







namespace qx_op{

class EMACalculator{
public:
    EMACalculator(int period):m_lastEMA(0.0),m_initialized(false){
        m_alpha = 2.0/(period + 1);
    }

    double Update(double value){
        if (!m_initialized) {
            m_lastEMA = value;
            m_initialized = true;
            return m_lastEMA;
        } else {
            m_lastEMA = m_alpha * value + (1 - m_alpha) * m_lastEMA;
        }
        return m_lastEMA;
    }

private:
    EMACalculator()=delete;
    EMACalculator(const EMACalculator &)=delete;



private:
    double m_lastEMA;
    bool   m_initialized;
    double m_alpha;
};



class DEMACalculator{
public:
    DEMACalculator(int period):m_ema1(0.0),m_ema2(0.0),m_initialized(false),m_period(period){
        m_alpha = 2.0/(period + 1);
    }

    double Update(double value){
        if (m_period <= 1) {
            return value;
        }

        if (!m_initialized) {
            m_ema1 = value;
            m_ema2 = value;
            m_initialized = true;
            return value;
        }

        m_ema1 = m_alpha * value + (1 - m_alpha) * m_ema1;
        m_ema2 = m_alpha * m_ema1 + (1 - m_alpha) * m_ema2;
        
        // DEMA公式: 2*EMA1 - EMA2
        return 2 * m_ema1 - m_ema2;
    }

private:
    DEMACalculator()=delete;
    DEMACalculator(const DEMACalculator &)=delete;


private:
    int    m_period;
    double m_ema1 = 0.0;  // 第一个EMA缓存
    double m_ema2 = 0.0;  // 第二个EMA缓存
    bool   m_initialized;
    double m_alpha;
};







}











